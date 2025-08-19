# Memory Allocation Helpers

The render subsystem exposes a small set of helpers built on top of
[Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
(VMA). These utilities centralise common allocation patterns and help
avoid fragmentation by funnelling allocations through shared helpers.

## Staging Buffers

```cpp
voxelvk::StagingBuffer buf;
createStagingBuffer(allocator, size, buf);
// write to buf.mapped then issue copies
vkCmdCopyBuffer(cmd, buf.buffer, gpuBuffer, 1, &copy);
```

Staging buffers are host visible and persistently mapped. Call
`destroyStagingBuffer` when finished.

## Upload and Readback

`uploadBuffer` and `readbackBuffer` provide one-shot utilities for
copying data between the host and device using a temporary staging
buffer.

## Transient Pool

`TransientBufferPool` lets render passes allocate scratch buffers that
are freed each frame:

```cpp
TransientBufferPool pool;
pool.init(allocator);
// each frame
pool.reset();
VkBuffer tmp; VmaAllocation alloc;
pool.allocate(size, usage, tmp, alloc);
```

Resetting the pool releases all tracked buffers, simplifying cleanup and
reducing fragmentation.
