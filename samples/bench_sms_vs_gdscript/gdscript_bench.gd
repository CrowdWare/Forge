extends SceneTree

func run_workload(iterations: int) -> int:
    var i: int = 0
    var acc: int = 0
    while i < iterations:
        var v: int = i * 1664525 + 1013904223
        acc += v % 2147483647
        i += 1
    return acc

func _initialize() -> void:
    var t0 := Time.get_ticks_usec()
    var result := run_workload(5000000)
    var t1 := Time.get_ticks_usec()
    var dt := t1 - t0

    print("RESULT:%d" % result)
    print("TIME_US:%d" % dt)
    quit()
