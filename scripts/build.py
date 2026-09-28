#!/usr/bin/env python3
"""
build.py - ProcessTest の選択ビルドラッパー（~/development/cmakeBuild.py のパターンを踏襲）

CI/CD の実行時間を削減するため、指定した APP（app/ 配下）だけをビルドする。
PF（firmware / vehicle_api）と契約面のテストは常にビルド・実行される。

使い方:
    python3 scripts/build.py                                  # 全 APP（ALL）
    python3 scripts/build.py --apps motor_control             # motor_control のみ
    python3 scripts/build.py --apps motor_control,hvac        # 複数指定
    python3 scripts/build.py --apps NONE                      # PF のみ（docs 変更時など）
    python3 scripts/build.py --changed                        # git 差分から対象 APP を自動判定
    python3 scripts/build.py --preset host-debug --test       # ビルド後に ctest 実行
    python3 scripts/build.py --preset host-sota --package dist  # SOTA 配布単位（APP .so）を出力
    python3 scripts/build.py --clean --apps hvac              # クリーン後に hvac のみ再ビルド
    python3 scripts/build.py --clean-only                     # build/ を削除のみ

--changed の判定規則:
    app/<name>/** または test/unit/app/<name>/** の変更 → その APP を対象に追加
    docs/**・*.md のみの変更                             → NONE（PF とテスト基盤のみ）
    上記以外（pf/・cmake/・ecu/・test 共通・ルート等）    → ALL（全体ビルド）
"""

import argparse
import shutil
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
BUILD_ROOT = ROOT_DIR / "build"
APP_DIR = ROOT_DIR / "app"


def run(cmd: list, cwd: Path = ROOT_DIR) -> None:
    print(f"[build.py] $ {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"[build.py] コマンドが失敗しました (exit code {result.returncode})")
        sys.exit(result.returncode)


def available_apps() -> list:
    """app/ 配下の CMakeLists.txt を持つディレクトリ = 選択可能な APP 一覧"""
    return sorted(p.name for p in APP_DIR.iterdir()
                  if p.is_dir() and (p / "CMakeLists.txt").exists())


def git_changed_files() -> list:
    """origin/main（無ければ main）との差分 + 未コミット変更のファイル一覧"""
    files: set = set()
    for base in ("origin/main", "main"):
        merge_base = subprocess.run(
            ["git", "merge-base", "HEAD", base],
            cwd=ROOT_DIR, capture_output=True, text=True)
        if merge_base.returncode == 0:
            diff = subprocess.run(
                ["git", "diff", "--name-only", merge_base.stdout.strip(), "HEAD"],
                cwd=ROOT_DIR, capture_output=True, text=True)
            files.update(diff.stdout.splitlines())
            break
    # 未コミットの変更（ローカル実行時の利便性のため）
    status = subprocess.run(["git", "status", "--porcelain"],
                            cwd=ROOT_DIR, capture_output=True, text=True)
    for line in status.stdout.splitlines():
        files.add(line[3:].split(" -> ")[-1])
    return sorted(f for f in files if f)


def detect_apps_from_changes() -> str:
    """変更ファイルからビルド対象 APP を決定する（判定規則はモジュール docstring 参照）"""
    changed = git_changed_files()
    if not changed:
        print("[build.py] --changed: 変更なし → NONE（PF のみ）")
        return "NONE"

    apps: set = set()
    full_build = False
    for f in changed:
        parts = f.split("/")
        if parts[0] == "app" and len(parts) > 1:
            apps.add(parts[1])
        elif f.startswith("test/unit/app/") and len(parts) > 3:
            apps.add(parts[3])
        elif parts[0] == "docs" or f.endswith(".md"):
            continue  # ドキュメントのみの変更はビルド対象に影響しない
        else:
            full_build = True  # pf/・cmake/・ecu/・共通テスト・ルート設定等

    if full_build:
        print(f"[build.py] --changed: 共通部の変更を検出 → ALL  (files={len(changed)})")
        return "ALL"
    if not apps:
        print("[build.py] --changed: docs のみの変更 → NONE（PF のみ）")
        return "NONE"
    known = set(available_apps())
    selected = sorted(apps & known)
    print(f"[build.py] --changed: 対象 APP = {selected}")
    return ",".join(selected) if selected else "NONE"


def normalize_apps(apps: str) -> str:
    """カンマ区切りを CMake リスト（セミコロン区切り）へ変換し、存在を検証する"""
    if apps in ("ALL", "NONE"):
        return apps
    names = [a.strip() for a in apps.split(",") if a.strip()]
    known = available_apps()
    unknown = [a for a in names if a not in known]
    if unknown:
        print(f"[build.py] 不明な APP: {unknown}（選択可能: {known}）")
        sys.exit(2)
    return ";".join(names)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="ProcessTest 選択ビルドラッパー",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=f"選択可能な APP: {', '.join(available_apps())}")
    parser.add_argument("--apps", default="ALL",
                        help="ビルド対象 APP（ALL / NONE / カンマ区切りリスト。デフォルト: ALL）")
    parser.add_argument("--changed", action="store_true",
                        help="git 差分からビルド対象 APP を自動判定する（--apps より優先）")
    parser.add_argument("--preset", default="host-coverage",
                        help="CMake configure/build プリセット（デフォルト: host-coverage）")
    parser.add_argument("--test", action="store_true",
                        help="ビルド後に ctest を実行する")
    parser.add_argument("--package", metavar="DIR",
                        help="APP を SOTA 配布単位（COMPONENT）ごとに DIR へ install する"
                             "（--preset host-sota 等の APPS_SHARED=ON ビルドで使用）")
    parser.add_argument("--clean", action="store_true",
                        help="build/ を削除してから実行する")
    parser.add_argument("--clean-only", action="store_true",
                        help="build/ の削除のみ行い、ビルドしない")
    args = parser.parse_args()

    if args.clean or args.clean_only:
        if BUILD_ROOT.exists():
            print(f"[build.py] クリーンアップ: {BUILD_ROOT} を削除します")
            shutil.rmtree(BUILD_ROOT)
        if args.clean_only:
            return

    apps = detect_apps_from_changes() if args.changed else args.apps
    build_apps = normalize_apps(apps)
    build_dir = BUILD_ROOT / args.preset

    # configure（プリセット + 選択 APP）→ build
    run(["cmake", "--preset", args.preset, f"-DBUILD_APPS={build_apps}"])
    run(["cmake", "--build", "--preset", args.preset, "-j"])

    if args.test:
        run(["ctest", "--test-dir", str(build_dir), "--output-on-failure"])

    if args.package:
        selected = (available_apps() if build_apps == "ALL"
                    else [] if build_apps == "NONE" else build_apps.split(";"))
        for app in selected:
            prefix = Path(args.package).resolve() / app
            run(["cmake", "--install", str(build_dir),
                 "--component", f"app_{app}", "--prefix", str(prefix)])
        print(f"[build.py] SOTA パッケージ出力先: {args.package}/<app>/lib/apps/")

    print(f"[build.py] 完了: BUILD_APPS={build_apps} / preset={args.preset}")


if __name__ == "__main__":
    main()
