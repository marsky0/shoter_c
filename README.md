# Shoter\_c

**Shoter\_c** is a high-performance C-based tool designed to detect rapid price impulses in market data according to user-defined parameters.
It uses the **HDF5** and **HDF5 High-Level (hdf5\_hl)** libraries to efficiently read compressed historical market datasets, enabling fast and memory-efficient processing.

The program scans data for sudden price movements over a given time window (`interval`), identifies impulses based on a volatility threshold (`trigger-delta`), and records all detected events in a `.csv` file for later analysis.
This makes it useful for traders, quants, and researchers who need to locate high-volatility moments for backtesting or signal development.

---

## Features

* **Efficient data access** using HDF5 for large compressed market datasets.
* **Customizable detection parameters** via command-line arguments.
* **High-speed processing** with the ability to select the number of CPU cores.
* **CSV output** for easy integration into analysis pipelines.
* **Cross-platform** (Linux, macOS, Windows with HDF5 support).

---

## Command-Line Usage

```bash
./shoter_c \
  --date-from "2025-07-01" \
  --date-to "2025-07-22" \
  --path-read-symbols symbols \
  --interval 50 \
  --trigger-delta 0.7 \
  --post-price-time 1000 \
  --cpu 1
```

---

## Parameters

| Parameter             | Description                                                                                  |
| --------------------- | -------------------------------------------------------------------------------------------- |
| `--date-from`         | Start date for processing (format: `YYYY-MM-DD`).                                            |
| `--date-to`           | End date for processing (format: `YYYY-MM-DD`).                                              |
| `--path-read-symbols` | Path to the folder with files containing the list of market symbols to be analyzed.          |
| `--interval`          | Time window (in milliseconds) used to measure the price impulse.                             |
| `--trigger-delta`     | Volatility threshold (in %) over the `interval` that triggers impulse detection.             |
| `--post-price-time`   | Time (in milliseconds) after an impulse to record the post-event price for later evaluation. |
| `--cpu`               | CPU core index to bind the process for optimized performance.                                |

---

## How It Works

1. **Data Loading** — Reads compressed market data using HDF5/HDF5\_HL.
2. **Impulse Detection** — For each symbol, the program checks price changes over the given `interval` and compares them to `trigger-delta`.
3. **Post-Impulse Tracking** — After detecting an impulse, it measures the price again after `post-price-time` milliseconds.
4. **Logging** — Each event is stored in a CSV file containing timestamps, price changes, and relevant metrics.

---

## Example CSV Output

```
symbol        start_price,         peak_price           post_price
BANANAS31USDT,0.012767000123858452,0.012876000255346298,0.012845999561250210
BANANAS31USDT,0.013283999636769295,0.013181000016629696,0.013199999928474426
BANANAS31USDT,0.014096000231802464,0.014217999763786793,0.014200000092387199
BANANAS31USDT,0.014383000321686268,0.014495000243186951,0.014453000078598658
BANANAS31USDT,0.013563999906182289,0.013435999862849712,0.013436999962185368

```

---
