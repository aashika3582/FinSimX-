import yfinance as yf
import pandas as pd
import os

def download_and_append_aapl():
    filename = "aapl_data.csv"
    ticker = "AAPL"

    print("Checking and downloading updated AAPL data...\n")

    # Load existing CSV if it exists
    if os.path.exists(filename):
        existing_data = pd.read_csv(filename)
        existing_data['Date'] = pd.to_datetime(existing_data['Date'])
        last_date = existing_data['Date'].max()
        start_date = (last_date + pd.Timedelta(days=1)).strftime('%Y-%m-%d')
        print(f"Last recorded date: {last_date.date()}")
    else:
        existing_data = pd.DataFrame()
        start_date = "2020-01-01"  # fallback if no file exists

    # Download new data only after the last recorded date
    data = yf.download(ticker, start=start_date, interval="1d", progress=False)
    if data.empty:
        print("No new data available.\n")
        return

    # Prepare new data
    data = data.reset_index()
    data['Date'] = data['Date'].dt.strftime('%Y-%m-%d')
    data = data[['Date', 'Open', 'High', 'Low', 'Close', 'Volume']]

    # Append to existing
    if not existing_data.empty:
        final_data = pd.concat([existing_data, data], ignore_index=True).drop_duplicates(subset='Date')
    else:
        final_data = data

    # Save updated CSV
    final_data.to_csv(filename, index=False)
    print(f"Updated AAPL data saved to {filename}")
    print(f"Latest available date: {final_data['Date'].iloc[-1]}")
    print(f"Total rows: {len(final_data)}\n")

if __name__ == "__main__":
    download_and_append_aapl()
