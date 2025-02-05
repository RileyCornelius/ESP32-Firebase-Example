#pragma once

// Toggle debug benchmarking
#ifndef DEBUG_BENCHMARK
#define DEBUG_BENCHMARK 1
#endif

// Benchmarking serial port
#ifndef DEBUG_BENCHMARK_SERIAL
#define DEBUG_BENCHMARK_SERIAL Serial
#endif

#if DEBUG_BENCHMARK
// Creates previous time with label, use same label with BENCHMARK_END(label) to print elapsed milliseconds
// label must be a symbol
#define BENCHMARK_BEGIN(label) I_BENCHMARK_BEGIN(CONCAT(_prevTime_, label))
#define I_BENCHMARK_BEGIN(prevTime) \
    static unsigned long prevTime;  \
    prevTime = millis();

// Prints elapsed time since BENCHMARK_BEGIN(label) in milliseconds
// label must be a symbol
#define BENCHMARK_END(label) I_BENCHMARK_END(CONCAT(_prevTime_, label), #label)
#define I_BENCHMARK_END(prevTime, label) DEBUG_BENCHMARK_SERIAL.printf("%s: %lu ms\n", label, millis() - prevTime);

// Creates previous time with label, use same label with BENCHMARK_MICROS_END(label) to print elapsed microseconds
// label must be a symbol
#define BENCHMARK_MICROS_BEGIN(label) I_BENCHMARK_MICROS_BEGIN(CONCAT(_prevTime_, label))
#define I_BENCHMARK_MICROS_BEGIN(prevTime) \
    static unsigned long prevTime;         \
    prevTime = micros();

// Prints elapsed time since BENCHMARK_MICROS_BEGIN(label) in microseconds
// label must be a symbol
#define BENCHMARK_MICROS_END(label) I_BENCHMARK_MICROS_END(CONCAT(_prevTime_, label), #label)
#define I_BENCHMARK_MICROS_END(prevTime, label) DEBUG_BENCHMARK_SERIAL.printf("%s: %lu us\n", label, micros() - prevTime);

#ifndef CONCAT
#define CONCAT(x, y) I_CONCAT(x, y)
#define I_CONCAT(x, y) x##y
#endif

#else
#define BENCHMARK_BEGIN(label)
#define BENCHMARK_END(label)

#define BENCHMARK_MICROS_BEGIN(label)
#define BENCHMARK_MICROS_END(label)

#endif