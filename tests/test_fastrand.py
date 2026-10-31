import sys
import threading

import fastrand


def test_pcg32_ranges():
    for _ in range(10000):
        assert 0 <= fastrand.pcg32bounded(1001) < 1001
        assert 100 <= fastrand.pcg32randint(100, 1000) <= 1000
        assert 0 <= fastrand.pcg32() < 2**32
        assert 0.0 <= fastrand.pcg32_uniform() < 1.0


def test_pcg32_seed_is_reproducible():
    fastrand.pcg32_seed(42)
    a = [fastrand.pcg32() for _ in range(10)]
    fastrand.pcg32_seed(42)
    b = [fastrand.pcg32() for _ in range(10)]
    assert a == b


def test_xorshift128plus():
    for _ in range(10000):
        assert 0 <= fastrand.xorshift128plus() < 2**64
        assert 0.0 <= fastrand.xorshift128plus_uniform() < 1.0
    if fastrand.SIXTYFOUR:
        for _ in range(10000):
            assert 0 <= fastrand.xorshift128plusbounded(1001) < 1001
            assert 100 <= fastrand.xorshift128plusrandint(100, 1000) <= 1000


def test_threads():
    errors = []

    def work():
        try:
            for _ in range(20000):
                assert 0 <= fastrand.pcg32bounded(1000) < 1000
                assert 0 <= fastrand.xorshift128plus() < 2**64
        except Exception as e:  # pragma: no cover
            errors.append(e)

    threads = [threading.Thread(target=work) for _ in range(8)]
    for t in threads:
        t.start()
    for t in threads:
        t.join()
    assert not errors


def test_gil_stays_disabled():
    # On free-threaded builds, importing fastrand must not re-enable the GIL.
    if hasattr(sys, "_is_gil_enabled") and "t" in getattr(sys, "abiflags", ""):
        assert not sys._is_gil_enabled()
