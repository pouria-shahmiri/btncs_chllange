# ========================================
# Databento DBN Reader → CSV Export
# ========================================
import databento as db
import pandas as pd

# --- 1. Read the DBN file ---
DBN_PATH = "./CLX5_mbo (2).dbn"

store = db.DBNStore.from_file(DBN_PATH)

# --- 2. Convert to pandas DataFrame ---
df = store.to_df()

print("✅ DBN file loaded successfully!")
print(f"Number of records: {len(df)}")
print("Columns:", df.columns.tolist())
print(df.head(5).to_string())  # preview first 5 rows

# --- 3. Sort by ts_event and add datetime column ---
df = df.sort_values(by="ts_event")
df["datetime"] = pd.to_datetime(df["ts_event"], unit="ns", utc=True)

# --- 4. Save full cleaned DataFrame to CSV ---
CSV_OUTPUT_PATH = "orderbook_clean.csv"
df.to_csv(CSV_OUTPUT_PATH, index=False)
print(f"✅ Saved full cleaned data to {CSV_OUTPUT_PATH}")

# --- 5. Optional: create summary per side & price ---
summary = (
    df.groupby(["side", "price"])["size"]
    .sum()
    .reset_index()
    .sort_values(by="price", ascending=False)
)

summary_csv_path = "orderbook_summary.csv"
summary.to_csv(summary_csv_path, index=False)
print(f"✅ Saved summary to {summary_csv_path}")

# --- 6. Print some statistics ---
num_symbols = df["symbol"].nunique()
print(f"Number of unique symbols: {num_symbols}")
print("Symbols in file:", df["symbol"].unique())

min_price = df["price"].min()
max_price = df["price"].max()
mean_price = df["price"].mean()
print(f"\nPrice statistics:")
print(f"Minimum price: {min_price}")
print(f"Maximum price: {max_price}")
print(f"Mean price: {mean_price:.2f}")

# --- 7. Show dtypes and first record transposed ---
print("\nData types of columns:")
print(df.dtypes)
print("\nFirst record (transposed):")
print(df.head(1).T)
