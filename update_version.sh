#!/bin/bash

# 現在のブランチ名を取得
BRANCH=$(git rev-parse --abbrev-ref HEAD)

# version.h から現在のバージョン名を取得
CURRENT_VERSION=$(sed -n 's/#define APP_VERSION "\([^"]*\)"/\1/p' version.h)

# もし CURRENT_VERSION が空なら、デフォルトで v0.0.0 を使用
if [ -z "$CURRENT_VERSION" ]; then
  CURRENT_VERSION="v0.0.0"
fi

# デフォルトのバージョンは現在の version.h のバージョンを使用
APP_VERSION=$CURRENT_VERSION

# ブランチごとの処理
if [[ $BRANCH == release/* ]]; then
    # release ブランチならバージョンを x.y.z に設定
    APP_VERSION=${BRANCH#release/}  # ブランチ名から "release/" を除去

elif [[ $BRANCH == feature/* ]]; then
    # feature ブランチなら、バージョンの後にフィーチャー名を付加
    FEATURE_NAME=${BRANCH#feature/}  # ブランチ名から "feature/" を除去
    APP_VERSION="$CURRENT_VERSION-$FEATURE_NAME"
fi

# 新しいバージョンが既存のバージョンと同じかどうかをチェック
if [ "$APP_VERSION" != "$CURRENT_VERSION" ]; then
    echo "#define APP_VERSION \"$APP_VERSION\"" > version.h
    echo "version.h has been updated to: $APP_VERSION"
else
    echo "version.h is up-to-date: $CURRENT_VERSION"
fi
