#!/system/bin/sh

resetprop=/data/adb/ksu/bin/resetprop
service=android.system.suspend-service

"$resetprop" -n suspend.short_suspend_backoff_enabled false

pid="$(pidof "$service")"
if [ -n "$pid" ]; then
	kill "$pid"
fi
