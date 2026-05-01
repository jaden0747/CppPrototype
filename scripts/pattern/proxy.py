"""
Proxy Pattern
=============
Intent: Provide a surrogate or placeholder for another object to control access.

Three variants:
  1. Virtual Proxy — lazy initialisation
  2. Protection Proxy — role-based access control
  3. Caching Proxy — memoises expensive calls
"""

from abc import ABC, abstractmethod
from enum import Enum, auto
from typing import Optional, Dict


# ---------------------------------------------------------------------------
# 1. Virtual (Lazy) Proxy
# ---------------------------------------------------------------------------

class Image(ABC):
    @abstractmethod
    def display(self) -> None: ...
    @abstractmethod
    def name(self) -> str: ...
    @abstractmethod
    def is_loaded(self) -> bool: ...


class RealImage(Image):
    def __init__(self, filename: str):
        self._filename = filename
        self._loaded = False
        self._load()

    def _load(self):
        self._loaded = True  # simulates disk I/O

    def display(self) -> None: pass  # simulate render
    def name(self)    -> str:  return self._filename
    def is_loaded(self) -> bool: return self._loaded


class LazyImageProxy(Image):
    def __init__(self, filename: str):
        self._filename = filename
        self._real: Optional[RealImage] = None

    def display(self) -> None:
        if self._real is None:
            self._real = RealImage(self._filename)
        self._real.display()

    def name(self)      -> str:  return self._filename
    def is_loaded(self) -> bool: return self._real is not None and self._real.is_loaded()


# ---------------------------------------------------------------------------
# 2. Protection Proxy
# ---------------------------------------------------------------------------

class Role(Enum):
    Guest = auto()
    User  = auto()
    Admin = auto()


class Service(ABC):
    @abstractmethod
    def get_data(self) -> str: ...
    @abstractmethod
    def delete_data(self) -> bool: ...


class RealService(Service):
    def get_data(self)    -> str:  return "Sensitive data"
    def delete_data(self) -> bool: return True


class ProtectionProxy(Service):
    def __init__(self, real: Service, role: Role):
        self._real = real
        self._role = role

    def get_data(self) -> str:
        if self._role == Role.Guest:
            raise PermissionError("Access denied: guests cannot read data")
        return self._real.get_data()

    def delete_data(self) -> bool:
        if self._role != Role.Admin:
            raise PermissionError("Access denied: only admins can delete")
        return self._real.delete_data()


# ---------------------------------------------------------------------------
# 3. Caching Proxy
# ---------------------------------------------------------------------------

class DataSource(ABC):
    @abstractmethod
    def fetch(self, key: str) -> str: ...


class SlowDataSource(DataSource):
    def __init__(self):
        self.fetch_count = 0

    def fetch(self, key: str) -> str:
        self.fetch_count += 1
        return f"value_of_{key}"


class CachingProxy(DataSource):
    def __init__(self, real: DataSource):
        self._real  = real
        self._cache: Dict[str, str] = {}

    def fetch(self, key: str) -> str:
        if key not in self._cache:
            self._cache[key] = self._real.fetch(key)
        return self._cache[key]

    @property
    def cache_size(self) -> int:
        return len(self._cache)


# ---------------------------------------------------------------------------
# Usage example
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    # Lazy proxy
    proxy = LazyImageProxy("vacation.jpg")
    print(f"Loaded before display: {proxy.is_loaded()}")
    proxy.display()
    print(f"Loaded after display:  {proxy.is_loaded()}")

    # Protection proxy
    for role in (Role.Guest, Role.User, Role.Admin):
        p = ProtectionProxy(RealService(), role)
        try:
            print(f"[{role.name}] getData: {p.get_data()}")
        except PermissionError as e:
            print(f"[{role.name}] {e}")

    # Caching proxy
    slow  = SlowDataSource()
    cache = CachingProxy(slow)
    cache.fetch("a"); cache.fetch("a"); cache.fetch("b")
    print(f"Real fetches: {slow.fetch_count}, Cache size: {cache.cache_size}")
