import 'dart:math' as math;
import 'dart:ui' show lerpDouble;

import 'package:flutter/cupertino.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter/widgets.dart';

class CicadaSlider extends StatefulWidget {
  const CicadaSlider({
    Key key,
    @required this.value,
    @required this.onChanged,
    this.bufferValue,
    this.onChangeStart,
    this.onChangeEnd,
    this.min = 0.0,
    this.max = 1.0,
    this.divisions,
    this.activeColor,
    this.trackColor,
    this.bufferColor,
    this.thumbColor = CupertinoColors.white,
  })  : assert(value != null),
        assert(min != null),
        assert(max != null),
        assert(value >= min && value <= max),
        assert(divisions == null || divisions > 0),
        assert(thumbColor != null),
        super(key: key);

  final double value;
  final double bufferValue;
  final ValueChanged<double> onChanged;
  final ValueChanged<double> onChangeStart;
  final ValueChanged<double> onChangeEnd;
  final double min;
  final double max;
  final int divisions;
  final Color activeColor;
  final Color trackColor;
  final Color bufferColor;
  final Color thumbColor;

  @override
  _CicadaSliderState createState() => _CicadaSliderState();

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties.add(DoubleProperty('value', value));
    properties.add(DoubleProperty('min', min));
    properties.add(DoubleProperty('max', max));
  }
}

class _CicadaSliderState extends State<CicadaSlider>
    with TickerProviderStateMixin {
  void _handleChanged(double value) {
    assert(widget.onChanged != null);
    final double lerpValue = lerpDouble(widget.min, widget.max, value);
    if (lerpValue != widget.value) {
      widget.onChanged(lerpValue);
    }
  }

  void _handleDragStart(double value) {
    assert(widget.onChangeStart != null);
    widget.onChangeStart(lerpDouble(widget.min, widget.max, value));
  }

  void _handleDragEnd(double value) {
    assert(widget.onChangeEnd != null);
    widget.onChangeEnd(lerpDouble(widget.min, widget.max, value));
  }

  @override
  Widget build(BuildContext context) {
    return _AliyunSliderRenderObjectWidget(
      value: (widget.value - widget.min) / (widget.max - widget.min),
      bufferValue:
          (widget.bufferValue - widget.min) / (widget.max - widget.min),
      divisions: widget.divisions,
      activeColor: CupertinoDynamicColor.resolve(
        widget.activeColor ?? CupertinoTheme.of(context).primaryColor,
        context,
      ),
      trackColor: CupertinoDynamicColor.resolve(
        widget.trackColor ?? CupertinoColors.systemFill,
        context,
      ),
      bufferColor: CupertinoDynamicColor.resolve(
        widget.bufferColor ?? CupertinoTheme.of(context).primaryColor,
        context,
      ),
      thumbColor: widget.thumbColor,
      onChanged: widget.onChanged != null ? _handleChanged : null,
      onChangeStart: widget.onChangeStart != null ? _handleDragStart : null,
      onChangeEnd: widget.onChangeEnd != null ? _handleDragEnd : null,
      vsync: this,
    );
  }
}

class _AliyunSliderRenderObjectWidget extends LeafRenderObjectWidget {
  const _AliyunSliderRenderObjectWidget({
    Key key,
    this.value,
    this.bufferValue,
    this.divisions,
    this.activeColor,
    this.trackColor,
    this.bufferColor,
    this.thumbColor,
    this.onChanged,
    this.onChangeStart,
    this.onChangeEnd,
    this.vsync,
  }) : super(key: key);

  final double value;
  final double bufferValue;
  final int divisions;
  final Color activeColor;
  final Color trackColor;
  final Color bufferColor;
  final Color thumbColor;
  final ValueChanged<double> onChanged;
  final ValueChanged<double> onChangeStart;
  final ValueChanged<double> onChangeEnd;
  final TickerProvider vsync;

  @override
  _RenderAliyunSlider createRenderObject(BuildContext context) {
    return _RenderAliyunSlider(
      value: value,
      bufferValue: bufferValue,
      divisions: divisions,
      activeColor: activeColor,
      bufferColor: bufferColor,
      thumbColor: CupertinoDynamicColor.resolve(thumbColor, context),
      // trackColor:
      //     CupertinoDynamicColor.resolve(CupertinoColors.systemFill, context),
      trackColor: trackColor,
      onChanged: onChanged,
      onChangeStart: onChangeStart,
      onChangeEnd: onChangeEnd,
      vsync: vsync,
      textDirection: Directionality.of(context),
    );
  }

  @override
  void updateRenderObject(
      BuildContext context, _RenderAliyunSlider renderObject) {
    renderObject
      ..value = value
      ..bufferValue = bufferValue
      ..divisions = divisions
      ..activeColor = activeColor
      ..thumbColor = CupertinoDynamicColor.resolve(thumbColor, context)
      ..trackColor = trackColor
      ..bufferColor = bufferColor
      // CupertinoDynamicColor.resolve(CupertinoColors.systemFill, context)
      ..onChanged = onChanged
      ..onChangeStart = onChangeStart
      ..onChangeEnd = onChangeEnd
      ..textDirection = Directionality.of(context);
  }
}

const double _kPadding = 8.0;
const double _kSliderHeight = 2.0 * (CupertinoThumbPainter.radius + _kPadding);
const double _kSliderWidth = 176.0; // Matches Material Design slider.
const Duration _kDiscreteTransitionDuration = Duration(milliseconds: 500);

const double _kAdjustmentUnit =
    0.1; // Matches iOS implementation of material slider.

class _RenderAliyunSlider extends RenderConstrainedBox {
  _RenderAliyunSlider({
    @required double value,
    double bufferValue,
    int divisions,
    Color activeColor,
    Color thumbColor,
    Color trackColor,
    Color bufferColor,
    ValueChanged<double> onChanged,
    this.onChangeStart,
    this.onChangeEnd,
    TickerProvider vsync,
    @required TextDirection textDirection,
  })  : assert(value != null && value >= 0.0 && value <= 1.0),
        assert(textDirection != null),
        _value = value,
        _bufferValue = bufferValue,
        _divisions = divisions,
        _activeColor = activeColor,
        _thumbColor = thumbColor,
        _trackColor = trackColor,
        _bufferColor = bufferColor,
        _onChanged = onChanged,
        _textDirection = textDirection,
        super(
            additionalConstraints: const BoxConstraints.tightFor(
                width: _kSliderWidth, height: _kSliderHeight)) {
    _drag = HorizontalDragGestureRecognizer()
      ..onStart = _handleDragStart
      ..onUpdate = _handleDragUpdate
      ..onEnd = _handleDragEnd;
    _position = AnimationController(
      value: value,
      duration: _kDiscreteTransitionDuration,
      vsync: vsync,
    )..addListener(markNeedsPaint);
  }

  double get value => _value;
  double _value;
  set value(double newValue) {
    assert(newValue != null && newValue >= 0.0 && newValue <= 1.0);
    if (newValue == _value) return;
    _value = newValue;
    if (divisions != null)
      _position.animateTo(newValue, curve: Curves.fastOutSlowIn);
    else
      _position.value = newValue;
    markNeedsSemanticsUpdate();
  }

  double get bufferValue => _bufferValue;
  double _bufferValue;
  set bufferValue(double value) {
    if (value == _bufferValue) return;
    _bufferValue = value;
    markNeedsPaint();
  }

  int get divisions => _divisions;
  int _divisions;
  set divisions(int value) {
    if (value == _divisions) return;
    _divisions = value;
    markNeedsPaint();
  }

  Color get activeColor => _activeColor;
  Color _activeColor;
  set activeColor(Color value) {
    if (value == _activeColor) return;
    _activeColor = value;
    markNeedsPaint();
  }

  Color get thumbColor => _thumbColor;
  Color _thumbColor;
  set thumbColor(Color value) {
    if (value == _thumbColor) return;
    _thumbColor = value;
    markNeedsPaint();
  }

  Color get trackColor => _trackColor;
  Color _trackColor;
  set trackColor(Color value) {
    if (value == _trackColor) return;
    _trackColor = value;
    markNeedsPaint();
  }

  Color get bufferColor => _bufferColor;
  Color _bufferColor;
  set bufferColor(Color value) {
    if (value == _bufferColor) return;
    _bufferColor = value;
    markNeedsPaint();
  }

  ValueChanged<double> get onChanged => _onChanged;
  ValueChanged<double> _onChanged;
  set onChanged(ValueChanged<double> value) {
    if (value == _onChanged) return;
    final bool wasInteractive = isInteractive;
    _onChanged = value;
    if (wasInteractive != isInteractive) markNeedsSemanticsUpdate();
  }

  ValueChanged<double> onChangeStart;
  ValueChanged<double> onChangeEnd;

  TextDirection get textDirection => _textDirection;
  TextDirection _textDirection;
  set textDirection(TextDirection value) {
    assert(value != null);
    if (_textDirection == value) return;
    _textDirection = value;
    markNeedsPaint();
  }

  AnimationController _position;

  HorizontalDragGestureRecognizer _drag;
  double _currentDragValue = 0.0;

  double get _discretizedCurrentDragValue {
    double dragValue = _currentDragValue.clamp(0.0, 1.0) as double;
    if (divisions != null)
      dragValue = (dragValue * divisions).round() / divisions;
    return dragValue;
  }

  double get _trackLeft => _kPadding;
  double get _trackRight => size.width - _kPadding;
  double get _thumbCenter {
    double visualPosition;
    switch (textDirection) {
      case TextDirection.rtl:
        visualPosition = 1.0 - _value;
        break;
      case TextDirection.ltr:
        visualPosition = _value;
        break;
    }
    return lerpDouble(_trackLeft + CupertinoThumbPainter.radius,
        _trackRight - CupertinoThumbPainter.radius, visualPosition);
  }

  double get _bufferRight {
    double visualPosition;
    switch (textDirection) {
      case TextDirection.rtl:
        visualPosition = 1.0 - _bufferValue - _value;
        break;
      case TextDirection.ltr:
        visualPosition = _bufferValue - _value;
        break;
    }
    return lerpDouble(_trackLeft + CupertinoThumbPainter.radius,
        _trackRight - CupertinoThumbPainter.radius, visualPosition);
  }

  bool get isInteractive => onChanged != null;

  void _handleDragStart(DragStartDetails details) =>
      _startInteraction(details.globalPosition);

  void _handleDragUpdate(DragUpdateDetails details) {
    if (isInteractive) {
      final double extent = math.max(_kPadding,
          size.width - 2.0 * (_kPadding + CupertinoThumbPainter.radius));
      final double valueDelta = details.primaryDelta / extent;
      switch (textDirection) {
        case TextDirection.rtl:
          _currentDragValue -= valueDelta;
          break;
        case TextDirection.ltr:
          _currentDragValue += valueDelta;
          break;
      }
      onChanged(_discretizedCurrentDragValue);
    }
  }

  void _handleDragEnd(DragEndDetails details) => _endInteraction();

  void _startInteraction(Offset globalPosition) {
    if (isInteractive) {
      if (onChangeStart != null) {
        onChangeStart(_discretizedCurrentDragValue);
      }
      _currentDragValue = _value;
      onChanged(_discretizedCurrentDragValue);
    }
  }

  void _endInteraction() {
    if (onChangeEnd != null) {
      onChangeEnd(_discretizedCurrentDragValue);
    }
    _currentDragValue = 0.0;
  }

  @override
  bool hitTestSelf(Offset position) {
    return (position.dx - _thumbCenter).abs() <
        CupertinoThumbPainter.radius + _kPadding;
  }

  @override
  void handleEvent(PointerEvent event, BoxHitTestEntry entry) {
    assert(debugHandleEvent(event, entry));
    if (event is PointerDownEvent && isInteractive) _drag.addPointer(event);
  }

  @override
  void paint(PaintingContext context, Offset offset) {
    double visualPosition;
    Color leftColor;
    Color rightColor;
    switch (textDirection) {
      case TextDirection.rtl:
        visualPosition = 1.0 - _position.value;
        leftColor = _activeColor;
        rightColor = trackColor;
        break;
      case TextDirection.ltr:
        visualPosition = _position.value;
        leftColor = trackColor;
        rightColor = _activeColor;
        break;
    }

    final double trackCenter = offset.dy + size.height / 2.0;
    final double trackLeft = offset.dx + _trackLeft;
    final double trackTop = trackCenter - 1.0;
    final double trackBottom = trackCenter + 1.0;
    final double trackRight = offset.dx + _trackRight;
    final double trackActive = offset.dx + _thumbCenter;
    final double buffingRight = trackActive + _bufferRight;

    final Canvas canvas = context.canvas;

    ///未播放进度
    if (visualPosition < 1.0) {
      final Paint paint = Paint()..color = leftColor;
      canvas.drawRRect(
          RRect.fromLTRBXY(
              trackActive, trackTop, trackRight, trackBottom, 1.0, 1.0),
          paint);
    }

    ///buffering进度
    if (visualPosition > 0.0 && bufferValue >= 0) {
      final Paint paint = Paint()..color = _bufferColor;
      //buffre end draw
      if (bufferValue >= 1.0) {
        canvas.drawRRect(
            RRect.fromLTRBXY(
                trackActive, trackTop, trackRight, trackBottom, 1.0, 1.0),
            paint);
      } else {
        canvas.drawRRect(
            RRect.fromLTRBXY(
                trackActive, trackTop, buffingRight, trackBottom, 1.0, 1.0),
            paint);
      }

      ///播放进度
      if (visualPosition > 0.0) {
        final Paint paint = Paint()..color = rightColor;
        canvas.drawRRect(
            RRect.fromLTRBXY(
                trackLeft, trackTop, trackActive, trackBottom, 1.0, 1.0),
            paint);
      }
    }

    if (visualPosition >= 1.0) {
      final Offset thumbCenter = Offset(trackRight, trackCenter);
      CupertinoThumbPainter(color: thumbColor).paint(
          canvas,
          Rect.fromCircle(
              center: thumbCenter, radius: CupertinoThumbPainter.radius));
    } else {
      final Offset thumbCenter = Offset(trackActive, trackCenter);
      CupertinoThumbPainter(color: thumbColor).paint(
          canvas,
          Rect.fromCircle(
              center: thumbCenter, radius: CupertinoThumbPainter.radius));
    }
  }

  @override
  void describeSemanticsConfiguration(SemanticsConfiguration config) {
    super.describeSemanticsConfiguration(config);

    config.isSemanticBoundary = isInteractive;
    if (isInteractive) {
      config.textDirection = textDirection;
      config.onIncrease = _increaseAction;
      config.onDecrease = _decreaseAction;
      config.value = '${(value * 100).round()}%';
      config.increasedValue =
          '${((value + _semanticActionUnit).clamp(0.0, 1.0) * 100).round()}%';
      config.decreasedValue =
          '${((value - _semanticActionUnit).clamp(0.0, 1.0) * 100).round()}%';
    }
  }

  double get _semanticActionUnit =>
      divisions != null ? 1.0 / divisions : _kAdjustmentUnit;

  void _increaseAction() {
    if (isInteractive)
      onChanged((value + _semanticActionUnit).clamp(0.0, 1.0) as double);
  }

  void _decreaseAction() {
    if (isInteractive)
      onChanged((value - _semanticActionUnit).clamp(0.0, 1.0) as double);
  }
}
