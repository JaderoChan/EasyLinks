APP="$1"
BUILD_DIR="$2"
DMG_NAME="${BUILD_DIR}/${APP}.dmg"

create-dmg \
  --volname "$APP" \
  --window-pos 200 120 \
  --window-size 600 360 \
  --icon-size 100 \
  --icon "${APP}.app" 150 180 \
  --app-drop-link 450 180 \
  "$DMG_NAME" \
  "$BUILD_DIR/${APP}.app"
