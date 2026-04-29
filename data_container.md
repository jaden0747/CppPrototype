
Design a sender and receiver ports to transfer data using a ring buffer (mempool) for following use case:

multiclient: 1 sender port can connect to multiple receiver ports, but each receiver port can only connect to one sender port. The sender port writes data into the ring buffer, and the receiver ports read data from the ring buffer. The sender port should be able to reverse the data before sending it to the receiver ports. The system should be designed to handle high throughput and low latency.

zero copy: the sender port should be able to write data directly into the ring buffer without copying it, and the receiver ports should be able to read data directly from the ring buffer without copying it. The system should be designed to minimize memory usage and maximize performance.

1. Multithreaded data transfer
2. Event driven updates: senderport can be used anytime to reserve memory and deliver
3. Minimal latency (not important right now)
4. Efficient memory management (not important right now)

My ideas:

- **Sender Port**:
  - Maintains a ring buffer (mempool) for outgoing data.
  - Provides an API to write data into the buffer, which can be called from multiple threads.
  - Uses atomic operations to manage buffer indices and ensure thread safety.
  - Notifies the receiver port when new data is available using an event or condition variable.
  - interfaces:
    - reverse(): to reverse the data in the buffer before sending
    - deliver(): to deliver the data to the receiver port
    - isConnected(): to check if at least 1 receiver port is connected
    - connectMempool(): connect to a mempool to use for data transfer
    - disconnectMempool(): disconnect from the mempool

- **Mempool**:
  - A fixed-size memory pool that manages a set of pre-allocated buffers.
  - Provides an API to allocate and deallocate buffers for data transfer.
  - Ensures efficient memory usage by reusing buffers and minimizing fragmentation.

- **Receiver Port**:
  - Update to check for new data in the ring buffer when notified by the sender port.
  - Provides an API to read data from the ring buffer, which can be called from multiple threads.
  - Uses atomic operations to manage buffer indices and ensure thread safety.
  - interfaces:
    - update(): to check if there is new data available in the buffer and read it.
    - cleanup(): clean up new data flag
    - hasData(): to check if there is data available in the buffer
    - hasNewData(): only available between update() and cleanup()
    - getData(): to read data from the buffer
    - isConnected(): to check if connected to a sender port

For the current code, some ideas for the use case:

1. all receiver ports will update at the beginning of the mainloop, then cleanup at the end of the mainloop.
2. sender ports and mempool are public and static so that everyone can access them. This is for event driven design (commands registry will be able to use sender ports to send data to receiver ports when there's new socket data coming).
