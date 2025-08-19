import threading
import time

from src.utils.async_io import AsyncIOPool


def test_threads_terminated_after_shutdown() -> None:
    """Threads spawned by the pool should terminate on shutdown."""
    before = {t.name for t in threading.enumerate()}
    with AsyncIOPool(max_workers=2) as pool:
        futures = [pool.submit(time.sleep, 0.01) for _ in range(2)]
        for fut in futures:
            fut.result()
    # Give the executor a moment to tear down threads
    time.sleep(0.05)
    after = {t.name for t in threading.enumerate()}
    assert all(not name.startswith("ThreadPoolExecutor") for name in after - before)
