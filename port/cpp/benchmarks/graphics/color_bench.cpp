// Migrated from src/Graphics/tests/Graphics.Benchmarks/ColorBenchmarker.cs (BenchmarkDotNet -> Google Benchmark).
#include <benchmark/benchmark.h>

#include "maui/graphics/color.hpp"

using maui::graphics::color;

static void bm_color_parse_hex(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto c = color::parse("#979797");
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_color_parse_hex);

static void bm_color_parse_black(benchmark::State &state)
{
    for (auto _ : state)
    {
        auto c = color::parse("Black");
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_color_parse_black);

// C#'s named-color lookup does not trim, so a name padded with spaces does not match and parse
// throws (faithful behavior). Guarded so the benchmark still runs and measures the attempt.
static void bm_color_parse_spaced_name(benchmark::State &state)
{
    for (auto _ : state)
    {
        color c;
        try
        {
            c = color::parse(" LightGoldenrodYellow ");
        }
        catch (...)
        {
        }
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(bm_color_parse_spaced_name);
