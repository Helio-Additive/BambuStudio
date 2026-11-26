#!/usr/bin/env bash
set -euo pipefail

app=$1
keychain="$RUNNER_TEMP/tester-signing.keychain-db"
p12="$RUNNER_TEMP/tester-signing.p12"
keychain_password=$(openssl rand -hex 32)
umask 077
trap 'rm -f "$p12"; security delete-keychain "$keychain" >/dev/null 2>&1 || true' EXIT
printf '%s' "$MACOS_CERTIFICATE" | base64 -D > "$p12"
security create-keychain -p "$keychain_password" "$keychain"
security set-keychain-settings -lut 3600 "$keychain"
security unlock-keychain -p "$keychain_password" "$keychain"
# codesign looks up the matching private key in the user search list.
security list-keychains -d user -s "$keychain"
security import "$p12" -k "$keychain" -P "$MACOS_CERTIFICATE_PWD" -T /usr/bin/codesign >/dev/null
rm "$p12"
security set-key-partition-list -S apple-tool:,apple: -s -k "$keychain_password" "$keychain" >/dev/null
identity=$(security find-identity -v -p codesigning "$keychain" | awk '/"Developer ID Application:/ {print $2}')
[[ "$identity" =~ ^[A-Fa-f0-9]{40}$ ]]
flags=(--force --options runtime --timestamp --sign "$identity")

codesign "${flags[@]}" --entitlements src/platform/osx/entitlements.plist "$app"
codesign --verify --deep --strict --verbose=2 "$app"
