# Simple-Signal-Slot

this is a simple signal-slot library, it's header-only.

## How to use

use template class `Signal<Args...>` to create a signal, the template arguments is slot's arguments.

use `Signal::connect(Slot slot)` to connect a slot.

you can connect the `lambda`, `std::function`, `funtor`. and it support `std::move` to move a rvlaue.

use template class `Slot<Args...>` to create a slot, the template arguments is slot's arguments.

use `operation()(arg...)` to emit a signal, it will call all the slot it connected.

it will return a template class `Connection<Args...>`, suggest use `auto` to get the return.

you can use member function `block` and `unblock` to block the slot be called.
and use the `disconnect()` to disconnect between signal and  slot.

## A test

```c++
struct A {
  void f(int x) { std::cout << "A::f(" << x << ")\n"; }
};

void signal_test() {
  // create a signal
  Signal<int> signal;

  Slot<int> slot1 = [](int x) { std::cout << "slot1(" << x << ")\n"; };

  A a;
  signal.connect(std::bind(&A::f, &a, std::placeholders::_1));
  signal.connect(slot1);
  auto conn =
      signal.connect([](int x) { std::cout << "lambda(" << x << ")\n"; });

  signal(1); // output: A::f(1) slot1(1) lambda(1)

  signal.disconnect(slot1);
  std::cout << "disconnect slot1\n";
  signal(2); // output: A::f(2) lambda(2)

  conn.block();
  std::cout << "block lambda\n";
  signal(3); // output: A::f(3)

  conn.unblock();
  std::cout << "unblock lambda\n";
  signal(4); // output: A::f(4) lambda(4)

  conn.disconnect();
  std::cout << "disconnect lambda\n";
  signal(5); // output: A::f(5)
}
```
