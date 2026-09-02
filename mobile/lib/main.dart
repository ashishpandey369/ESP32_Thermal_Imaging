import 'dart:async';
import 'dart:io';
import 'dart:math' as math;
import 'dart:typed_data';

import 'package:flutter/material.dart';

void main() => runApp(const ThermalApp());

class ThermalApp extends StatelessWidget {
  const ThermalApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'ESP32 Thermal Imaging',
      theme: ThemeData.dark(useMaterial3: true),
      home: const ThermalHome(),
    );
  }
}

class ThermalFrame {
  final int number;
  final Float32List pixels;
  final double min;
  final double max;
  final double average;
  final double center;

  ThermalFrame({
    required this.number,
    required this.pixels,
    required this.min,
    required this.max,
    required this.average,
    required this.center,
  });
}

class ThermalHome extends StatefulWidget {
  const ThermalHome({super.key});

  @override
  State<ThermalHome> createState() => _ThermalHomeState();
}

class _ThermalHomeState extends State<ThermalHome> {
  static const int magic = 0x54484D4C;
  static const int frameSize = 3108;
  static const int headerSize = 16;
  static const int pixelCount = 768;

  final hostController = TextEditingController(text: '192.168.4.1');
  final portController = TextEditingController(text: '8080');

  Socket? socket;
  StreamSubscription<List<int>>? subscription;
  final List<int> buffer = <int>[];
  ThermalFrame? frame;

  bool connecting = false;
  String status = 'Disconnected';
  int receivedFrames = 0;
  int droppedFrames = 0;
  DateTime? fpsStart;
  double fps = 0;
  int? lastFrame;

  @override
  void dispose() {
    disconnect();
    hostController.dispose();
    portController.dispose();
    super.dispose();
  }

  Future<void> connect() async {
    if (connecting) return;
    await disconnect();
    setState(() {
      connecting = true;
      status = 'Connecting...';
      frame = null;
    });

    try {
      final host = hostController.text.trim();
      final port = int.parse(portController.text.trim());
      final s = await Socket.connect(
        host,
        port,
        timeout: const Duration(seconds: 5),
      );
      s.setOption(SocketOption.tcpNoDelay, true);
      socket = s;
      buffer.clear();
      fpsStart = DateTime.now();
      receivedFrames = 0;
      droppedFrames = 0;
      lastFrame = null;
      setState(() {
        connecting = false;
        status = 'Connected';
      });

      subscription = s.listen(
        onData,
        onError: (_) => _connectionLost('Connection error'),
        onDone: () => _connectionLost('Disconnected'),
        cancelOnError: true,
      );
    } catch (e) {
      setState(() {
        connecting = false;
        status = 'Connection failed: $e';
      });
    }
  }

  Future<void> disconnect() async {
    await subscription?.cancel();
    subscription = null;
    await socket?.close();
    socket = null;
  }

  void _connectionLost(String message) {
    socket = null;
    subscription = null;
    if (mounted) setState(() => status = message);
  }

  void onData(List<int> data) {
    buffer.addAll(data);

    // Process every complete binary frame currently buffered. If the UI is
    // briefly busy, old frames are discarded so the display catches up to
    // the newest sensor frame instead of building latency.
    while (true) {
      if (buffer.length < headerSize) return;

      int start = _findMagic();
      if (start < 0) {
        if (buffer.length > 3) {
          buffer.removeRange(0, buffer.length - 3);
        }
        return;
      }

      if (start > 0) {
        buffer.removeRange(0, start);
      }

      if (buffer.length < frameSize) return;
      if (buffer[4] != 1 || _readU16LE(buffer, 6) != headerSize) {
        buffer.removeAt(0);
        continue;
      }

      final packet = Uint8List.fromList(buffer.sublist(0, frameSize));
      buffer.removeRange(0, frameSize);
      _parseFrame(packet);
    }
  }

  int _findMagic() {
    for (int i = 0; i <= buffer.length - 4; i++) {
      if (_readU32LE(buffer, i) == magic) return i;
    }
    return -1;
  }

  int _readU16LE(List<int> bytes, int offset) =>
      bytes[offset] | (bytes[offset + 1] << 8);

  int _readU32LE(List<int> bytes, int offset) =>
      bytes[offset] |
      (bytes[offset + 1] << 8) |
      (bytes[offset + 2] << 16) |
      (bytes[offset + 3] << 24);

  void _parseFrame(Uint8List packet) {
    final view = ByteData.sublistView(packet);
    final frameNumber = view.getUint32(8, Endian.little);
    final pixelOffset = headerSize;
    final statsOffset = pixelOffset + pixelCount * 4;
    final crcOffset = statsOffset + 16;

    final expectedCrc = view.getUint32(crcOffset, Endian.little);
    final actualCrc = crc32(packet, 0, crcOffset);
    if (expectedCrc != actualCrc) return;

    final pixels = Float32List(pixelCount);
    for (int i = 0; i < pixelCount; i++) {
      pixels[i] = view.getFloat32(pixelOffset + i * 4, Endian.little);
    }

    if (lastFrame != null && frameNumber > lastFrame! + 1) {
      droppedFrames += frameNumber - lastFrame! - 1;
    }
    lastFrame = frameNumber;
    receivedFrames++;

    final elapsed = DateTime.now()
        .difference(fpsStart ?? DateTime.now())
        .inMilliseconds;
    if (elapsed > 0) {
      fps = receivedFrames * 1000.0 / elapsed;
    }

    final newFrame = ThermalFrame(
      number: frameNumber,
      pixels: pixels,
      min: view.getFloat32(statsOffset, Endian.little),
      max: view.getFloat32(statsOffset + 4, Endian.little),
      average: view.getFloat32(statsOffset + 8, Endian.little),
      center: view.getFloat32(statsOffset + 12, Endian.little),
    );

    if (mounted) {
      setState(() => frame = newFrame);
    }
  }

  int crc32(Uint8List data, int start, int length) {
    int crc = 0xFFFFFFFF;
    for (int i = start; i < length; i++) {
      crc ^= data[i];
      for (int bit = 0; bit < 8; bit++) {
        crc = (crc & 1) != 0
            ? (crc >> 1) ^ 0xEDB88320
            : crc >> 1;
      }
    }
    return (~crc) & 0xFFFFFFFF;
  }

  @override
  Widget build(BuildContext context) {
    final f = frame;
    return Scaffold(
      backgroundColor: const Color(0xFF07090D),
      appBar: AppBar(
        title: const Text('ESP32 Thermal Imaging'),
        backgroundColor: const Color(0xFF0D1118),
        actions: [
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16),
            child: Center(
              child: Row(children: [
                Icon(
                  Icons.circle,
                  size: 11,
                  color: status == 'Connected'
                      ? Colors.greenAccent
                      : Colors.redAccent,
                ),
                const SizedBox(width: 8),
                Text(status, overflow: TextOverflow.ellipsis),
              ]),
            ),
          ),
        ],
      ),
      body: SafeArea(
        child: LayoutBuilder(
          builder: (context, constraints) {
            final landscape = constraints.maxWidth > constraints.maxHeight;
            final viewer = ThermalViewer(frame: f);
            final panel = _controlPanel(f);
            return landscape
                ? Row(children: [
                    Expanded(child: viewer),
                    SizedBox(width: 300, child: panel),
                  ])
                : Column(children: [
                    Expanded(child: viewer),
                    SizedBox(height: 285, child: panel),
                  ]);
          },
        ),
      ),
    );
  }

  Widget _controlPanel(ThermalFrame? f) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: const BoxDecoration(color: Color(0xFF0D1118)),
      child: SingleChildScrollView(
        child: Column(crossAxisAlignment: CrossAxisAlignment.stretch, children: [
          const Text(
            'WI-FI CONNECTION',
            style: TextStyle(
              color: Colors.lightBlueAccent,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 10),
          TextField(
            controller: hostController,
            decoration: const InputDecoration(
              labelText: 'ESP32 IP',
              border: OutlineInputBorder(),
            ),
          ),
          const SizedBox(height: 8),
          TextField(
            controller: portController,
            keyboardType: TextInputType.number,
            decoration: const InputDecoration(
              labelText: 'TCP Port',
              border: OutlineInputBorder(),
            ),
          ),
          const SizedBox(height: 10),
          FilledButton.icon(
            onPressed: connecting || status == 'Connected'
                ? disconnect
                : connect,
            icon: Icon(
              status == 'Connected' ? Icons.link_off : Icons.wifi,
            ),
            label: Text(
              status == 'Connected' ? 'Disconnect' : 'Connect',
            ),
          ),
          const SizedBox(height: 16),
          const Text(
            'LIVE DATA',
            style: TextStyle(
              color: Colors.lightBlueAccent,
              fontWeight: FontWeight.bold,
            ),
          ),
          const SizedBox(height: 8),
          _stat('Frame', f?.number.toString() ?? '-'),
          _stat('FPS', f == null ? '-' : fps.toStringAsFixed(1)),
          _stat('Min', f == null ? '-' : '${f.min.toStringAsFixed(2)} °C'),
          _stat('Max', f == null ? '-' : '${f.max.toStringAsFixed(2)} °C'),
          _stat('Average', f == null ? '-' : '${f.average.toStringAsFixed(2)} °C'),
          _stat('Center', f == null ? '-' : '${f.center.toStringAsFixed(2)} °C'),
          _stat('Dropped', droppedFrames.toString()),
        ]),
      ),
    );
  }

  Widget _stat(String name, String value) => Padding(
        padding: const EdgeInsets.symmetric(vertical: 3),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(name),
            Text(value, style: const TextStyle(fontWeight: FontWeight.w600)),
          ],
        ),
      );
}

class ThermalViewer extends StatelessWidget {
  final ThermalFrame? frame;
  const ThermalViewer({super.key, required this.frame});

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: Colors.black,
        borderRadius: BorderRadius.circular(10),
      ),
      clipBehavior: Clip.antiAlias,
      child: frame == null
          ? const Center(
              child: Text(
                'Connect to ESP32-Thermal',
                style: TextStyle(color: Colors.white54),
              ),
            )
          : CustomPaint(
              painter: ThermalPainter(frame!),
              child: const SizedBox.expand(),
            ),
    );
  }
}

class ThermalPainter extends CustomPainter {
  final ThermalFrame frame;
  ThermalPainter(this.frame);

  @override
  void paint(Canvas canvas, Size size) {
    final range = math.max(0.01, frame.max - frame.min);
    final paint = Paint()..filterQuality = FilterQuality.none;

    // Bilinear interpolation gives a much smoother visual image while the
    // underlying measurements remain the native 32x24 MLX90640 pixels.
    for (int py = 0; py < size.height.ceil(); py++) {
      final gy = py * 23.0 / math.max(1.0, size.height - 1.0);
      final y0 = gy.floor().clamp(0, 23);
      final y1 = math.min(23, y0 + 1);
      final fy = gy - y0;

      for (int px = 0; px < size.width.ceil(); px++) {
        final gx = px * 31.0 / math.max(1.0, size.width - 1.0);
        final x0 = gx.floor().clamp(0, 31);
        final x1 = math.min(31, x0 + 1);
        final fx = gx - x0;

        final v00 = frame.pixels[y0 * 32 + x0];
        final v10 = frame.pixels[y0 * 32 + x1];
        final v01 = frame.pixels[y1 * 32 + x0];
        final v11 = frame.pixels[y1 * 32 + x1];
        final top = v00 + (v10 - v00) * fx;
        final bottom = v01 + (v11 - v01) * fx;
        final value = top + (bottom - top) * fy;
        final t = ((value - frame.min) / range).clamp(0.0, 1.0);

        paint.color = inferno(t);
        canvas.drawRect(
          Rect.fromLTWH(px.toDouble(), py.toDouble(), 1.0, 1.0),
          paint,
        );
      }
    }
  }

  Color inferno(double t) {
    const stops = [
      Color(0xFF05000F),
      Color(0xFF2A0A5E),
      Color(0xFF741B6E),
      Color(0xFFBC3754),
      Color(0xFFED6925),
      Color(0xFFFBAE22),
      Color(0xFFFCFFA4),
    ];
    final p = t * (stops.length - 1);
    final i = p.floor().clamp(0, stops.length - 2);
    final f = p - i;
    return Color.lerp(stops[i], stops[i + 1], f)!;
  }

  @override
  bool shouldRepaint(covariant ThermalPainter oldDelegate) =>
      oldDelegate.frame.number != frame.number;
}
