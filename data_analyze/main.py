# ========================================
# Databento DBN Reader & Analyzer (no plots)
# ========================================
import databento as db
import pandas as pd

# --- 1. Read the DBN file ---
DBN_PATH = "./CLX5_mbo (2).dbn"

store = db.DBNStore.from_file(DBN_PATH)

# --- 2. Convert to pandas DataFrame ---
df = store.to_df()

# --- 3. Basic info ---
print("✅ File loaded successfully!")
print(f"Number of records: {len(df)}")
print("Columns:", df.columns.tolist())
# MODIFICATION: Use to_string() to display the first 5 rows completely
print(df.head(5).to_string())  # preview
# Original line: print(df.head(10)) 

# --- 4. Clean and sort ---
df = df.sort_values(by="ts_event")
df["datetime"] = pd.to_datetime(df["ts_event"], unit="ns")

# --- 5. Example analysis: total volume per price & side ---
summary = (
    df.groupby(["side", "price"])["size"]
    .sum()
    .reset_index()
    .sort_values(by="price", ascending=False)
)

print("\nTop 10 price levels by total size:")
print(summary.head(10))

# --- 6. Save cleaned data for future processing ---
df.to_parquet("orderbook_clean.parquet")
summary.to_csv("orderbook_summary.csv", index=False)

print("- orderbook_summary.csv")


num_symbols = df["symbol"].nunique()
print(f"Number of unique symbols: {num_symbols}")

print("Symbols in file:", df["symbol"].unique())


# --- Price stats ---
min_price = df["price"].min()
max_price = df["price"].max()
mean_price = df["price"].mean()

print(f"\nPrice statistics:")
print(f"Minimum price: {min_price}")
print(f"Maximum price: {max_price}")
print(f"Mean price: {mean_price:.2f}")