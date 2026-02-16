import 'package:flutter/material.dart';

typedef DigitCallback = void Function(String digit);

typedef BackspaceCallback = void Function();

typedef SubmitCallback = void Function();

typedef ClearCallback = void Function();

class NexgenKeypad extends StatelessWidget {
  const NexgenKeypad({
    super.key,
    required this.onDigit,
    required this.onBackspace,
    required this.onSubmit,
    required this.onClear,
  });

  final DigitCallback onDigit;
  final BackspaceCallback onBackspace;
  final SubmitCallback onSubmit;
  final ClearCallback onClear;

  Widget _btn(BuildContext context, {
    required Widget child,
    VoidCallback? onPressed,
  }) {
    return SizedBox(
      height: 64,
      child: FilledButton.tonal(
        onPressed: onPressed,
        child: child,
      ),
    );
  }

  Widget _digit(BuildContext context, String d) {
    return _btn(
      context,
      child: Text(d, style: const TextStyle(fontSize: 22, fontWeight: FontWeight.w700)),
      onPressed: () => onDigit(d),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        GridView.count(
          crossAxisCount: 3,
          shrinkWrap: true,
          physics: const NeverScrollableScrollPhysics(),
          mainAxisSpacing: 10,
          crossAxisSpacing: 10,
          children: [
            _digit(context, '1'),
            _digit(context, '2'),
            _digit(context, '3'),
            _digit(context, '4'),
            _digit(context, '5'),
            _digit(context, '6'),
            _digit(context, '7'),
            _digit(context, '8'),
            _digit(context, '9'),
            _btn(
              context,
              child: const Icon(Icons.backspace_outlined),
              onPressed: onBackspace,
            ),
            _digit(context, '0'),
            _btn(
              context,
              child: const Icon(Icons.check_circle_outline),
              onPressed: onSubmit,
            ),
          ],
        ),
        const SizedBox(height: 12),
        SizedBox(
          width: double.infinity,
          height: 52,
          child: OutlinedButton(
            onPressed: onClear,
            child: const Text('Clear'),
          ),
        ),
      ],
    );
  }
}
