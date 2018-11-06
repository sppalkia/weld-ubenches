
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <iostream>

#include "weld.h"

using namespace std;

#define COLUMNS (200 * 1000 * 1000)

const char *PROGRAM = "\
|\
  c1_values: vec[i32],\
  c1_nulls: vec[bool],\
  c2_values: vec[i32],\
  c2_nulls: vec[bool],\
  c3_values: vec[i32],\
  c3_nulls: vec[bool],\
  c4_values: vec[i32],\
  c4_nulls: vec[bool]\
|\
\
result(\
    for(\
      zip(c1_values, c1_nulls, c2_values, c2_nulls, c3_values, c3_nulls, c4_values, c4_nulls),\
      merger[i64,+],\
      |b, i, e|\
      if(e.$1 == false,\
      if(e.$3 == false,\
      if(e.$5 == false,\
      if(e.$1 == false && e.$0 < 0,\
        if(e.$3 == false && e.$2 < 0,\
          if(e.$5 == false && e.$4 < 0,\
            if(e.$7 == false && e.$6 < 0,\
              merge(b, 1L), b\
              ),\
            b\
            ),\
          b\
          ),\
        b\
        ),\
      b),\
      b),\
      b)\
      )\
    )";

// Static module/contexts.
weld_module_t module;
weld_context_t context;

template <typename T>
class weldvec {
public:
  T *ptr;
  int64_t length;

  weldvec(T *ptr, int64_t length) : ptr(ptr), length(length) {}
};

template <typename T>
class column {
public:

  long count;
  T *values;
  uint8_t *nulls;

  column(long size, int multiplier) {
    values = (T *)malloc(sizeof(T) * size);

    for (long i = 0; i < size; i++) {
      values[i] = (T)rand() * multiplier;
    }
    nulls = (uint8_t *)calloc(sizeof(T), size);
  }


  ~column() {
    free(values);
    free(nulls);
  }

};

// Forward declaration.
class weld_dataset;

class dataset {
public:
  long count;
  column<int32_t> c1;
  column<int32_t> c2;
  column<int32_t> c3;
  column<int32_t> c4;

  weld_dataset *weld_data;

  dataset(long size) :
    count(size),
    c1(column<int32_t>(size, 1)),
    c2(column<int32_t>(size, 10)),
    c3(column<int32_t>(size, 100)),
    c4(column<int32_t>(size, 1000)),
    weld_data(NULL) {}
};

class weld_dataset {
public:
  weldvec<int32_t> c1;
  weldvec<uint8_t> c1_nulls;
  weldvec<int32_t> c2;
  weldvec<uint8_t> c2_nulls;
  weldvec<int32_t> c3;
  weldvec<uint8_t> c3_nulls;
  weldvec<int32_t> c4;
  weldvec<uint8_t> c4_nulls;

  weld_dataset(dataset &d) :
    c1(weldvec<int32_t>(d.c1.values, d.count)),
    c1_nulls(weldvec<uint8_t>(d.c1.nulls, d.count)),
    c2(weldvec<int32_t>(d.c2.values, d.count)),
    c2_nulls(weldvec<uint8_t>(d.c2.nulls, d.count)),
    c3(weldvec<int32_t>(d.c3.values, d.count)),
    c3_nulls(weldvec<uint8_t>(d.c3.nulls, d.count)),
    c4(weldvec<int32_t>(d.c4.values, d.count)),
    c4_nulls(weldvec<uint8_t>(d.c4.nulls, d.count)) {}
};

typedef long (*benchmark)(dataset *);

int driver(benchmark b, const char *name, dataset *data) {

  long result;
  double average = 0;

  const int trials = 10;
  double times[trials];

  std::cout << name << std::endl;
  for (int i = 0; i < trials + 1; i++) {
    std::clock_t start, end;

    start = std::clock();
    result = b(data);
    end = std::clock();

    double runtime = (end - start) / (double)(CLOCKS_PER_SEC / 1000);

    if (i != 0) {
      times[i] = runtime;
      average += runtime;
    }

    std::cout
      << "\tTime: "
      << runtime
      << " ms"
      << ", Result: "
      << result
      << std::endl;

  }

  std::cout
    << "Average runtime: "
    << (average / (double)trials)
    << std::endl;

  return result;
}

// Implementations of the benchmark.
long standard(dataset *data) {
  long count = 0;
  for (long i = 0; i < data->count; i++) {
    if (data->c1.nulls[i] == 0 && data->c1.values[i] < 0) {
      if (data->c2.nulls[i] == 0 && data->c2.values[i] < 0) {
        if (data->c3.nulls[i] == 0 && data->c3.values[i] < 0) {
          if (data->c4.nulls[i] == 0 && data->c4.values[i] < 0) {
            count++;
          }
        }
      }
    }
  }
  return count;
}

long preload(dataset *data) {
  long count = 0;

  for (long i = 0; i < data->count; i++) {
    int32_t c1v, c2v, c3v, c4v;
    uint8_t c1n, c2n, c3n, c4n;
    c1v = data->c1.values[i];
    c1n = data->c1.nulls[i];
    c2v = data->c2.values[i];
    c2n = data->c2.nulls[i];
    c3v = data->c3.values[i];
    c3n = data->c3.nulls[i];
    c4v = data->c4.values[i];
    c4n = data->c4.nulls[i];
    if (c1n == 0 && c1v < 0) {
      if (c2n == 0 && c2v < 0) {
        if (c3n == 0 && c3v < 0) {
          if (c4n == 0 && c4v < 0) {
            count++;
          }
        }
      }
    }
  }
  return count;
}

long weld(dataset *data) {

  weld_error_t e = weld_error_new();
  weld_value_t arg = weld_value_new(data->weld_data);
  weld_value_t result = weld_module_run(module, context, arg, e);
  void *result_data = weld_value_data(result);
  long value = *(int64_t *)result_data;

  weld_value_free(arg);
  weld_value_free(result);
  weld_error_free(e);

  return value;
}

void compile_weld() {

    weld_error_t e = weld_error_new();
    weld_conf_t conf = weld_conf_new();

    // 10GB
    weld_conf_set(conf, "weld.memory.limit", "10000000000");

    module = weld_module_compile(PROGRAM, conf, e);
    context = weld_context_new(conf);

    weld_conf_free(conf);

    if (weld_error_code(e)) {
        const char *err = weld_error_message(e);
        printf("Error message: %s\n", err);
        exit(1);
    }

    weld_error_free(e);
}

int main(int argc, char **argv) {

  char *endptr;
  long columns;
  if (argc > 1) {
    columns = strtol(argv[1], &endptr, 10);
    assert(endptr != argv[1]);
  } else {
    columns = COLUMNS;
  }

  std::cout << "Generating data...";
  fflush(stdout);
  dataset d(columns);

  std::cout << "Setting up Weld...";
  fflush(stdout);
  weld_dataset wd(d);
  d.weld_data = &wd;
  compile_weld();
  std::cout << "done." << std::endl;
  fflush(stdout);

  struct benchmark_entry {
    benchmark b;
    const char *name;
  };

  const benchmark_entry benchmarks[] = {
    {standard, "standard"},
    {preload, "preload"},
    {weld, "weld"},
    {NULL, NULL}
  };

  int j = 0;
  while (benchmarks[j].b != NULL) {
    driver(benchmarks[j].b, benchmarks[j].name, &d);
    j++;
  }
}
