"""
Observer Pattern
================
Intent: Define a one-to-many dependency between objects so that when one
object changes state, all its dependents are notified and updated
automatically.

Real-world analogy: Newspaper subscriptions — subscribers receive new
editions automatically when published; they can cancel at any time.
"""

from __future__ import annotations
from abc import ABC, abstractmethod
from typing import List, Callable, Dict, TypeVar, Generic

# ---------------------------------------------------------------------------
# Observer / Subject interfaces
# ---------------------------------------------------------------------------

class Event:
    def __init__(self, type_: str, data: str):
        self.type = type_
        self.data = data

    def __repr__(self):
        return f"Event(type={self.type!r}, data={self.data!r})"


class IObserver(ABC):
    @abstractmethod
    def on_event(self, event: Event) -> None: ...


class ISubject(ABC):
    @abstractmethod
    def attach(self, observer: IObserver) -> None: ...

    @abstractmethod
    def detach(self, observer: IObserver) -> None: ...

    @abstractmethod
    def notify(self, event: Event) -> None: ...


# ---------------------------------------------------------------------------
# Concrete Subject
# ---------------------------------------------------------------------------

class StockMarket(ISubject):
    def __init__(self):
        self._observers: List[IObserver] = []
        self.last_ticker = ""
        self.last_price  = 0.0

    def attach(self, observer: IObserver) -> None:
        self._observers.append(observer)

    def detach(self, observer: IObserver) -> None:
        self._observers.remove(observer)

    def notify(self, event: Event) -> None:
        for obs in list(self._observers):
            obs.on_event(event)

    def set_price(self, ticker: str, price: float) -> None:
        self.last_ticker = ticker
        self.last_price  = price
        self.notify(Event(ticker, str(price)))

    @property
    def subscriber_count(self) -> int:
        return len(self._observers)


# ---------------------------------------------------------------------------
# Concrete Observers
# ---------------------------------------------------------------------------

class Logger(IObserver):
    def __init__(self):
        self.log: List[str] = []

    def on_event(self, event: Event) -> None:
        self.log.append(f"[{event.type}] {event.data}")


class AlertMonitor(IObserver):
    def __init__(self, threshold: float):
        self.threshold = threshold
        self.alerts: List[str] = []

    def on_event(self, event: Event) -> None:
        price = float(event.data)
        if price > self.threshold:
            self.alerts.append(f"{event.type} exceeded threshold: {event.data}")


# ---------------------------------------------------------------------------
# Generic EventEmitter (callback-based, no class hierarchy needed)
# ---------------------------------------------------------------------------

T = TypeVar("T")


class EventEmitter(Generic[T]):
    def __init__(self):
        self._handlers: Dict[int, Callable[[T], None]] = {}
        self._next_id = 0

    def subscribe(self, handler: Callable[[T], None]) -> int:
        sub_id = self._next_id
        self._next_id += 1
        self._handlers[sub_id] = handler
        return sub_id

    def unsubscribe(self, sub_id: int) -> None:
        self._handlers.pop(sub_id, None)

    def emit(self, value: T) -> None:
        for handler in list(self._handlers.values()):
            handler(value)

    @property
    def subscriber_count(self) -> int:
        return len(self._handlers)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    market  = StockMarket()
    logger  = Logger()
    monitor = AlertMonitor(200.0)

    market.attach(logger)
    market.attach(monitor)

    market.set_price("AAPL", 150.0)
    market.set_price("TSLA", 250.0)

    print("Log:",    logger.log)
    print("Alerts:", monitor.alerts)

    # EventEmitter demo
    emitter: EventEmitter[str] = EventEmitter()
    received: List[str] = []
    emitter.subscribe(lambda s: received.append(s))
    emitter.emit("hello")
    print("Received:", received)
