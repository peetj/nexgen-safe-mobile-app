import 'package:flutter_test/flutter_test.dart';

import 'package:nexgen_safe/main.dart';

void main() {
  testWidgets('renders Nexgen Safe home screen', (WidgetTester tester) async {
    await tester.pumpWidget(const NexgenSafeApp());

    expect(find.text('Nexgen Safe'), findsOneWidget);
    expect(find.text('Connect'), findsOneWidget);
  });
}
