using System.Diagnostics;

static long RunWorkload(int iterations)
{
    long i = 0;
    long acc = 0;
    while (i < iterations)
    {
        long v = i * 1664525L + 1013904223L;
        acc += v % 2147483647L;
        i++;
    }
    return acc;
}

var sw = Stopwatch.StartNew();
long result = RunWorkload(5_000_000);
sw.Stop();

double us = sw.Elapsed.TotalMilliseconds * 1000.0;
Console.WriteLine($"RESULT:{result}");
Console.WriteLine($"TIME_US:{us:0}");
