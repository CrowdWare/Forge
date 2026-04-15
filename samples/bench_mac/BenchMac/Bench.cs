// C# Benchmark — CrowdWare Forge 2026
// Compiled: dotnet Release /p:Optimize=false (Roslyn optimizer disabled)
// Exact same algorithm as bench.sms

using System;
using System.Diagnostics;

static class Bench {
    static long Fib(long n) {
        if (n <= 1L) return n;
        return Fib(n - 1L) + Fib(n - 2L);
    }

    static long LcgChain(long seed, long count) {
        long acc = seed;
        long i = 0L;
        while (i < count) {
            acc = (acc * 1664525L + 1013904223L) % 1000000007L;
            i = i + 1L;
        }
        return acc;
    }

    static long NestedMod(long n) {
        long sum = 0L, i = 0L;
        while (i < n) {
            long j = 1L;
            while (j <= n) {
                sum = (sum + i * j) % 999983L;
                j = j + 1L;
            }
            i = i + 1L;
        }
        return sum;
    }

    static void Main() {
        var sw = Stopwatch.StartNew();
        long r1 = Fib(36L);
        long r2 = LcgChain(42L, 2_000_000L);
        long r3 = NestedMod(800L);
        long result = r1 + r2 + r3;
        sw.Stop();
        double us = sw.Elapsed.TotalMicroseconds;
        Console.WriteLine($"RESULT:{result}");
        Console.WriteLine($"TIME_US:{us:F1}");
    }
}
