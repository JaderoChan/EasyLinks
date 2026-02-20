APP="$1"
BUILD_DIR="$2"
DMG_NAME="${APP}.dmg"
DMG_TEMP_DIR="dmg_temp"

echo "Creating DMG..."

# 清理旧文件
rm -rf "$DMG_TEMP_DIR" "$DMG_NAME"

# 创建临时目录结构
mkdir -p "$DMG_TEMP_DIR"
cp -r "$BUILD_DIR/${APP}.app" "$DMG_TEMP_DIR/"

# 创建 DMG
hdiutil create -volname "$APP" \
  -srcfolder "$DMG_TEMP_DIR" \
  -ov -format UDZO \
  -imagekey zlib-level=9 \
  "$DMG_NAME"

# 清理
rm -rf "$DMG_TEMP_DIR"
