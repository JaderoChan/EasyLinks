APP_TITLE="$1"
ICONS_FILE="$2"
BUILD_DIR="$3"

create-dmg \
  --volname "$APP_TITLE" \
  --window-pos 400 200 \
  --window-size 600 400 \
  --icon-size 125 \
  --volicon "${ICONS_FILE}" \
  --icon "${APP_TITLE}.app" 125 150 \
  --app-drop-link 475 150 \
  "${BUILD_DIR}/${APP_TITLE}.dmg" \
  "${BUILD_DIR}/${APP_TITLE}.app"
