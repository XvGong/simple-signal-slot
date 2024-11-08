#pragma once
#include <functional>

struct ConnectionSignalImpl {
  virtual ~ConnectionSignalImpl() = default;
  virtual void conn_disconnect(void *ptr) = 0;
  virtual void conn_block(void *ptr) = 0;
  virtual void conn_unblock(void *ptr) = 0;
  virtual bool conn_valid(void *ptr) const = 0;
  virtual bool conn_isBlocked(void *ptr) const = 0;
};

template <class... Args>
using Slot = std::function<void(Args...)>;

template <class... Args>
class Connection {
  ConnectionSignalImpl *signal = nullptr;
  void *ptr = nullptr;

public:
  Connection() = default;
  Connection(ConnectionSignalImpl &signal, void *ptr)
      : signal(&signal), ptr(ptr) {}
  Connection(const Connection &) = default;
  Connection(Connection &&) = default;
  Connection &operator=(const Connection &) = default;
  Connection &operator=(Connection &&) = default;

  bool isVaild() { return signal && ptr; }

  void disconnect() {
    signal->conn_disconnect(ptr);
    signal = nullptr;
    ptr = nullptr;
  }

  void block() { signal->conn_block(ptr); }

  void unblock() { signal->conn_unblock(ptr); }
};

template <class... Args>
class Signal : public ConnectionSignalImpl {
  struct slot {
    Slot<Args...> func;
    void *ptr;
    bool block = false;

    slot(Slot<Args...> &func, void *ptr, bool block = false)
        : func(func), ptr(ptr), block(block) {}

    slot(Slot<Args...> &&func, bool block = false)
        : func(std::move(func)), ptr(reinterpret_cast<void *>(&this->func)),
          block(block) {}
  };

  std::vector<slot> slots;

  void conn_disconnect(void *ptr) override {
    slots.erase(
        std::remove_if(slots.begin(), slots.end(),
                       [ptr](const Signal::slot &s) { return s.ptr == ptr; }),
        slots.end());
  }

  void conn_block(void *ptr) override {
    for (auto &slot : slots) {
      if (slot.ptr == ptr) {
        slot.block = true;
      }
    }
  }

  void conn_unblock(void *ptr) override {
    for (auto &slot : slots) {
      if (slot.ptr == ptr) {
        slot.block = false;
      }
    }
  }

  bool conn_valid(void *ptr) const override {
    for (auto &slot : slots) {
      if (slot.ptr == ptr) {
        return true;
      }
    }
    return false;
  }

  bool conn_isBlocked(void *ptr) const override {
    for (auto &slot : slots) {
      if (slot.ptr == ptr) {
        return slot.block;
      }
    }
    return false;
  }

public:
  // 传入左值对象，使用该对象指针标记
  Connection<Args...> connect(Slot<Args...> &slot) {
    slots.emplace_back(slot, reinterpret_cast<void *>(&slot), false);
    return Connection<Args...>(*this, slots.back().ptr);
  }
  // 传入右值对象，使用移动后的对象指针标记
  Connection<Args...> connect(Slot<Args...> &&slot) {
    slots.emplace_back(std::move(slot), false);
    return Connection<Args...>(*this, slots.back().ptr);
  }

  void disconnect(Slot<Args...> &slot) {
    auto ptr = reinterpret_cast<void *>(&slot);
    conn_disconnect(ptr);
  }

  void operator()(Args... args) {
    for (auto &slot : slots) {
      if (!slot.block) {
        slot.func(args...);
      }
    }
  }
};