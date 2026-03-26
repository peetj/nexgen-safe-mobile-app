enum NexgenTransport { wifiAp, ble, demo }

extension NexgenTransportLabels on NexgenTransport {
  String get title {
    switch (this) {
      case NexgenTransport.wifiAp:
        return 'ESP32 Wi-Fi';
      case NexgenTransport.ble:
        return 'Bluetooth LE';
      case NexgenTransport.demo:
        return 'Demo Mode';
    }
  }

  String get description {
    switch (this) {
      case NexgenTransport.wifiAp:
        return 'Recommended for school use. Join the safe hotspot and control it at 192.168.4.1.';
      case NexgenTransport.ble:
        return 'Direct BLE control. Useful when Bluetooth is preferred on Android.';
      case NexgenTransport.demo:
        return 'Runs the full UI with no hardware attached.';
    }
  }
}
