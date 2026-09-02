import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:math' as math;

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
  final List<double> pixels;
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
  final hostController = TextEditingController(text: '192.168.4.1');
  final portController = TextEditingController(text: '8080');

  Socket? socket;
  StreamSubscription<List<int>>? subscription;
  final StringBuffer buffer = StringBuffer();
  Map<String, dynamic>? header;
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
    });

    try {
      final host = hostController.text.trim();
      final port = int.parse(portController.text.trim());
      final s = await Socket.connect(host, port, timeout: const Duration(seconds: 5));
      socket = s;
      buffer.clear();
      header = null;
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
        status = 'Connection failed';
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
    buffer.write(utf8.decode(data, allowMalformed: true));
    final text = buffer.toString();
    final lines = text.split('\n');
    buffer
      ..clear()
      ..write(lines.removeLast());

    for (final raw in lines) {
      parseLine(raw.trim());
    }
  }

  void parseLine(String line) {
    if (line.isEmpty) return;
    final parts = line.split(',');

    if (parts.first == 'FRAME' && parts.length == 7) {
      try {
        header = {
          'number': int.parse(parts[1]),
          'min': double.parse(parts[3]),
          'max': double.parse(parts[4]),
          'average': double.parse(parts[5]),
          'center': double.parse(parts[6]),
        };
      } catch (_) {
        header = null;
      }
      return;
    }

    if (parts.first != 'DATA' || parts.length != 769 || header == null) return;

    try {
      final values = <double>[];
      for (var i = 1; i < parts.length; i++) {
        values.add(double.parse(parts[i]));
      }
      final h = header!;
      final number = h['number'] as int;
      if (lastFrame != null && number > lastFrame! + 1) {
        droppedFrames += number - lastFrame! - 1;
      }
      lastFrame = number;
      receivedFrames++;
      final elapsed = DateTime.now().difference(fpsStart ?? DateTime.now()).inMilliseconds;
      if (elapsed > 0) fps = receivedFrames * 1000.0 / elapsed;

      final newFrame = ThermalFrame(
        number: number,
        pixels: values,
        min: h['min'] as double,
        max: h['max'] as double,
        average: h['average'] as double,
        center: h['center'] as double,
      );
      header = null;
      if (mounted) setState(() => frame = newFrame);
    } catch (_) {
      header = null;
    }
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
                Icon(Icons.circle, size: 11, color: status == 'Connected' ? Colors.greenAccent : Colors.redAccent),
                const SizedBox(width: 8),
                Text(status),
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
                ? Row(children: [Expanded(child: viewer), SizedBox(width: 300, child: panel)])
                : Column(children: [Expanded(child: viewer), SizedBox(height: 285, child: panel)]);
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
          const Text('WI-FI CONNECTION', style: TextStyle(color: Colors.lightBlueAccent, fontWeight: FontWeight.bold)),
          const SizedBox(height: 10),
          TextField(controller: hostController, decoration: const InputDecoration(labelText: 'ESP32 IP', border: OutlineInputBorder())),
          const SizedBox(height: 8),
          TextField(controller: portController, keyboardType: TextInputType.number, decoration: const InputDecoration(labelText: 'TCP Port', border: OutlineInputBorder())),
          const SizedBox(height: 10),
          FilledButton.icon(
            onPressed: connecting || status == 'Connected' ? disconnect : connect,
            icon: Icon(status == 'Connected' ? Icons.link_off : Icons.wifi),
            label: Text(status == 'Connected' ? 'Disconnect' : 'Connect'),
          ),
          const SizedBox(height: 16),
          const Text('LIVE DATA', style: TextStyle(color: Colors.lightBlueAccent, fontWeight: FontWeight.bold)),
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
        child: Row(mainAxisAlignment: MainAxisAlignment.spaceBetween, children: [Text(name), Text(value, style: const TextStyle(fontWeight: FontWeight.w600))]),
      );
}

class ThermalViewer extends StatelessWidget {
  final ThermalFrame? frame;
  const ThermalViewer({super.key, required this.frame});

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.all(12),
      decoration: BoxDecoration(color: Colors.black, borderRadius: BorderRadius.circular(10)),
      clipBehavior: Clip.antiAlias,
      child: frame == null
          ? const Center(child: Text('Connect to ESP32-Thermal', style: TextStyle(color: Colors.white54)))
          : CustomPaint(painter: ThermalPainter(frame!), child: const SizedBox.expand()),
    );
  }
}

class ThermalPainter extends CustomPainter {
  final ThermalFrame frame;
  ThermalPainter(this.frame);

  @override
  void paint(Canvas canvas, Size size) {
    final cellW = size.width / 32.0;
    final cellH = size.height / 24.0;
    final range = math.max(0.01, frame.max - frame.min);
    final paint = Paint()..style = PaintingStyle.fill;

    for (var y = 0; y < 24; y++) {
      for (var x = 0; x < 32; x++) {
        final t = ((frame.pixels[y * 32 + x] - frame.min) / range).clamp(0.0, 1.0);
        paint.color = inferno(t);
        canvas.drawRect(Rect.fromLTWH(x * cellW, y * cellH, cellW + 0.5, cellH + 0.5), paint);
      }
    }
  }

  Color inferno(double t) {
    // Compact Inferno-like gradient: black/purple -> red -> orange -> yellow/white.
    const stops = [
      Color(0xFF05000F), Color(0xFF2A0A5E), Color(0xFF741B6E),
      Color(0xFFBC3754), Color(0xFFED6925), Color(0xFFFBAE22), Color(0xFFFCFFA4),
    ];
    final p = t * (stops.length - 1);
    final i = p.floor().clamp(0, stops.length - 2);
    final f = p - i;
    return Color.lerp(stops[i], stops[i + 1], f)!;
  }

  @override
  bool shouldRepaint(covariant ThermalPainter oldDelegate) => oldDelegate.frame.number != frame.number;
}
