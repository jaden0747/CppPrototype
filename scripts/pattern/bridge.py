"""
Bridge Pattern
==============
Intent: Decouple an abstraction from its implementation so the two can vary
independently.

Real-world analogy: A remote control (abstraction) works with any device
(implementation) — TV, Radio, etc. Advanced remotes extend the abstraction
without touching the device hierarchy.
"""

from abc import ABC, abstractmethod


# ---------------------------------------------------------------------------
# Implementor interface
# ---------------------------------------------------------------------------

class Device(ABC):
    @abstractmethod
    def is_enabled(self) -> bool: ...
    @abstractmethod
    def enable(self) -> None: ...
    @abstractmethod
    def disable(self) -> None: ...
    @abstractmethod
    def volume(self) -> int: ...
    @abstractmethod
    def set_volume(self, percent: int) -> None: ...
    @abstractmethod
    def channel(self) -> int: ...
    @abstractmethod
    def set_channel(self, ch: int) -> None: ...
    @abstractmethod
    def name(self) -> str: ...


# ---------------------------------------------------------------------------
# Concrete Implementors
# ---------------------------------------------------------------------------

class TV(Device):
    def __init__(self):
        self._enabled = False
        self._volume  = 30
        self._channel = 1

    def is_enabled(self) -> bool:   return self._enabled
    def enable(self)     -> None:   self._enabled = True
    def disable(self)    -> None:   self._enabled = False
    def volume(self)     -> int:    return self._volume
    def set_volume(self, v: int) -> None: self._volume = max(0, min(100, v))
    def channel(self)    -> int:    return self._channel
    def set_channel(self, ch: int) -> None: self._channel = ch
    def name(self)       -> str:    return "TV"


class Radio(Device):
    def __init__(self):
        self._enabled = False
        self._volume  = 50
        self._channel = 1

    def is_enabled(self) -> bool:   return self._enabled
    def enable(self)     -> None:   self._enabled = True
    def disable(self)    -> None:   self._enabled = False
    def volume(self)     -> int:    return self._volume
    def set_volume(self, v: int) -> None: self._volume = max(0, min(100, v))
    def channel(self)    -> int:    return self._channel
    def set_channel(self, ch: int) -> None: self._channel = ch
    def name(self)       -> str:    return "Radio"


# ---------------------------------------------------------------------------
# Abstraction
# ---------------------------------------------------------------------------

class RemoteControl:
    def __init__(self, device: Device):
        self._device = device

    @property
    def device(self) -> Device:
        return self._device

    def toggle_power(self):
        if self._device.is_enabled():
            self._device.disable()
        else:
            self._device.enable()

    def volume_down(self): self._device.set_volume(self._device.volume() - 10)
    def volume_up(self):   self._device.set_volume(self._device.volume() + 10)
    def channel_down(self): self._device.set_channel(self._device.channel() - 1)
    def channel_up(self):   self._device.set_channel(self._device.channel() + 1)


# ---------------------------------------------------------------------------
# Refined Abstraction
# ---------------------------------------------------------------------------

class AdvancedRemote(RemoteControl):
    def mute(self):
        self._device.set_volume(0)

    def set_channel(self, ch: int):
        self._device.set_channel(ch)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    tv     = TV()
    radio  = Radio()

    remote         = RemoteControl(tv)
    advanced_radio = AdvancedRemote(radio)

    remote.toggle_power()
    remote.volume_up()
    print(f"{remote.device.name()}: enabled={remote.device.is_enabled()}, vol={remote.device.volume()}")

    advanced_radio.toggle_power()
    advanced_radio.set_channel(5)
    advanced_radio.mute()
    print(f"{advanced_radio.device.name()}: ch={advanced_radio.device.channel()}, vol={advanced_radio.device.volume()}")
