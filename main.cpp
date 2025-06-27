#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

using namespace std;

struct PriceData {
    string date;
    double open, high, low, close, volume;
    
    PriceData() : open(0), high(0), low(0), close(0), volume(0) {}
};

struct TradingSignal {
    string date;
    string action;
    double price;
    double rsi;
    double macd;
    double signal;
    double portfolio_value;
    string reason;
};

struct MACDResult {
    vector<double> macd;
    vector<double> signal;
    vector<double> histogram;
};

class TechnicalIndicators {
public:
    static vector<double> calculateSMA(const vector<double>& prices, int period) {
        vector<double> sma(prices.size(), 0.0);
        if (prices.size() < (size_t)period) return sma;
        
        for (size_t i = period - 1; i < prices.size(); ++i) {
            double sum = 0.0;
            for (int j = 0; j < period; ++j) {
                sum += prices[i - j];
            }
            sma[i] = sum / period;
        }
        return sma;
    }
    static vector<double> calculateEMA(const vector<double>& prices, int period) {
        vector<double> ema(prices.size(), 0.0);
        if (prices.empty()) return ema;
        
        double multiplier = 2.0 / (period + 1);
        ema[0] = prices[0];
        
        for (size_t i = 1; i < prices.size(); ++i) {
            ema[i] = (prices[i] * multiplier) + (ema[i - 1] * (1 - multiplier));
        }
        
        return ema;
    }
    
    static vector<double> calculateRSI(const vector<double>& prices, int period = 14) {
        vector<double> rsi(prices.size(), 50.0);
        if (prices.size() <= (size_t)period) return rsi;
        
        vector<double> gains, losses;
        for (size_t i = 1; i < prices.size(); ++i) {
            double change = prices[i] - prices[i - 1];
            gains.push_back(change > 0 ? change : 0);
            losses.push_back(change < 0 ? -change : 0);
        }
        
        double avg_gain = 0.0, avg_loss = 0.0;
        for (int i = 0; i < period; ++i) {
            avg_gain += gains[i];
            avg_loss += losses[i];
        }
        avg_gain /= period;
        avg_loss /= period;
        
        for (size_t i = period; i < prices.size(); ++i) {
            if (i - 1 < gains.size()) {
                avg_gain = (avg_gain * (period - 1) + gains[i - 1]) / period;
                avg_loss = (avg_loss * (period - 1) + losses[i - 1]) / period;
                
                if (avg_loss == 0) {
                    rsi[i] = 100.0;
                } else {
                    rsi[i] = 100.0 - (100.0 / (1.0 + (avg_gain / avg_loss)));
                }
            }
        }
        
        return rsi;
    }
    
    static MACDResult calculateMACD(const vector<double>& prices, int fast_period = 12, int slow_period = 26, int signal_period = 9) {
        
        vector<double> ema_fast = calculateEMA(prices, fast_period);
        vector<double> ema_slow = calculateEMA(prices, slow_period);
        vector<double> macd(prices.size(), 0.0);
        
        for (size_t i = 0; i < prices.size(); ++i) {
            macd[i] = ema_fast[i] - ema_slow[i];
        }
        
        vector<double> signal = calculateEMA(macd, signal_period);
        vector<double> histogram(prices.size());
        
        for (size_t i = 0; i < prices.size(); ++i) {
            histogram[i] = macd[i] - signal[i];
        }
        
        MACDResult result;
        result.macd = macd;
        result.signal = signal;
        result.histogram = histogram;
        
        return result;
    }
    
    static vector<double> calculateBollingerBands(const vector<double>& prices, int period = 20, double std_dev = 2.0) {
        vector<double> bb_lower(prices.size(), 0.0);
        
        for (size_t i = period - 1; i < prices.size(); ++i) {
            double sum = 0.0;
            for (int j = 0; j < period; ++j) {
                sum += prices[i - j];
            }
            double sma = sum / period;
            
            double variance = 0.0;
            for (int j = 0; j < period; ++j) {
                variance += pow(prices[i - j] - sma, 2);
            }
            double std = sqrt(variance / period);
            
            bb_lower[i] = sma - (std * std_dev);
        }
        
        return bb_lower;
    }
};

class CSVParser {
public:
    static vector<PriceData> loadPriceData(const string& filename) {
        vector<PriceData> data;
        ifstream file(filename.c_str());
        
        if (!file.is_open()) {
            throw runtime_error("Cannot open file: " + filename);
        }
        
        string line;
        bool first_line = true;
        
        while (getline(file, line)) {
            if (first_line) {
                first_line = false;
                continue;
            }
            
            if (line.empty()) continue;
            
            try {
                PriceData price_data = parseLine(line);
                if (price_data.close > 0) {
                    data.push_back(price_data);
                }
            } catch (const exception& e) {
                cerr << "Error parsing line: " << line << " - " << e.what() << endl;
            }
        }
        
        if (data.empty()) {
            throw runtime_error("No valid price data found in file");
        }
        
        return data;
    }
    
private:
    static PriceData parseLine(const string& line) {
        PriceData data;
        stringstream ss(line);
        string field;
        int field_count = 0;
        
        while (getline(ss, field, ',')) {
            try {
                switch (field_count) {
                    case 0: data.date = field; break;
                    case 1: data.open = atof(field.c_str()); break;
                    case 2: data.high = atof(field.c_str()); break;
                    case 3: data.low = atof(field.c_str()); break;
                    case 4: data.close = atof(field.c_str()); break;
                    case 5: data.volume = atof(field.c_str()); break;
                }
                field_count++;
            } catch (const exception& e) {
                throw runtime_error("Invalid number format in field: " + field);
            }
        }
        
        if (field_count < 5) {
            throw runtime_error("Insufficient fields in CSV line");
        }
        
        return data;
    }
};

class AdvancedTradingStrategy {
private:
    double cash;
    double shares;
    double initial_capital;
    vector<TradingSignal> trade_history;
    
    // Strategy parameters
    double rsi_oversold = 25.0;      // More aggressive oversold
    double rsi_overbought = 75.0;    // More aggressive overbought
    double rsi_neutral_low = 45.0;   // RSI neutral zone
    double rsi_neutral_high = 55.0;  // RSI neutral zone
    
    // Risk management
    double stop_loss_pct = 0.08;     // 8% stop loss
    double take_profit_pct = 0.12;   // 12% take profit
    double position_size_pct = 0.90; // Use 90% of available cash
    
    double entry_price = 0.0;
    int consecutive_losses = 0;
    int max_consecutive_losses = 3;
    
public:
    AdvancedTradingStrategy(double initial_cash = 100000.0) 
        : cash(initial_cash), shares(0.0), initial_capital(initial_cash) {}
    
    void backtest(const vector<PriceData>& price_data) {
        if (price_data.size() < 50) {
            throw runtime_error("Insufficient data for backtesting");
        }
        
        vector<double> prices;
        for (const auto& data : price_data) {
            prices.push_back(data.close);
        }
        
        // Calculate indicators
        vector<double> rsi = TechnicalIndicators::calculateRSI(prices, 14);
        vector<double> rsi_short = TechnicalIndicators::calculateRSI(prices, 7);  // Faster RSI
        MACDResult macd_result = TechnicalIndicators::calculateMACD(prices, 12, 26, 9);
        vector<double> sma_20 = TechnicalIndicators::calculateSMA(prices, 20);
        vector<double> sma_50 = TechnicalIndicators::calculateSMA(prices, 50);
        vector<double> bb_lower = TechnicalIndicators::calculateBollingerBands(prices, 20, 2.0);
        
        cout << "Starting backtest with " << price_data.size() << " data points..." << endl;
        cout << "Initial capital: $" << initial_capital << endl << endl;
        
        // Start trading after indicators stabilize
        for (size_t i = 50; i < price_data.size(); ++i) {
            double current_price = prices[i];
            double current_rsi = rsi[i];
            double current_rsi_short = rsi_short[i];
            double current_macd = macd_result.macd[i];
            double current_signal = macd_result.signal[i];
            double prev_macd = macd_result.macd[i-1];
            double prev_signal = macd_result.signal[i-1];
            
            // Trend detection
            bool uptrend = sma_20[i] > sma_50[i];
            bool macd_bullish = current_macd > current_signal;
            bool macd_crossover_up = (prev_macd <= prev_signal) && (current_macd > current_signal);
            bool macd_crossover_down = (prev_macd >= prev_signal) && (current_macd < current_signal);
            
            string action = "HOLD";
            string reason = "";
            
            // Risk management - Stop loss and take profit
            if (shares > 0 && entry_price > 0) {
                double current_return = (current_price - entry_price) / entry_price;
                
                // Stop loss
                if (current_return <= -stop_loss_pct) {
                    executeTrade(price_data[i], "SELL", "Stop Loss", current_rsi, current_macd, current_signal);
                    consecutive_losses++;
                    continue;
                }
                // Take profit
                if (current_return >= take_profit_pct) {
                    executeTrade(price_data[i], "SELL", "Take Profit", current_rsi, current_macd, current_signal);
                    consecutive_losses = 0;
                    continue;
                }
            }
            
            // BUY SIGNALS (Multiple conditions for higher probability)
            if (shares == 0 && cash > current_price && consecutive_losses < max_consecutive_losses) {
                
                // Signal 1: Strong oversold with MACD confirmation
                if (current_rsi < rsi_oversold && current_rsi_short < 30 && 
                    macd_bullish && uptrend) {
                    executeTrade(price_data[i], "BUY", "Strong Oversold + MACD Bull + Uptrend", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
                
                // Signal 2: MACD bullish crossover with RSI confirmation
                if (macd_crossover_up && current_rsi > 30 && current_rsi < 60 && uptrend) {
                    executeTrade(price_data[i], "BUY", "MACD Crossover + RSI Neutral + Uptrend", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
                
                // Signal 3: Bounce from Bollinger Band lower with RSI oversold
                if (current_price <= bb_lower[i] * 1.02 && current_rsi < 35 && 
                    current_price > prices[i-1]) {  // Price bouncing up
                    executeTrade(price_data[i], "BUY", "BB Lower Bounce + RSI Oversold", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
                
                // Signal 4: Simple RSI oversold in uptrend
                if (current_rsi < 30 && uptrend && macd_result.histogram[i] > macd_result.histogram[i-1]) {
                    executeTrade(price_data[i], "BUY", "RSI Oversold + Uptrend + MACD Improving", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
            }
            
            // SELL SIGNALS
            if (shares > 0) {
                
                // Signal 1: RSI overbought with MACD bearish
                if (current_rsi > rsi_overbought && !macd_bullish) {
                    executeTrade(price_data[i], "SELL", "RSI Overbought + MACD Bearish", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
                
                // Signal 2: MACD bearish crossover
                if (macd_crossover_down && current_rsi > 50) {
                    executeTrade(price_data[i], "SELL", "MACD Bearish Crossover", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
                
                // Signal 3: Trend reversal (SMA crossover down)
                if (!uptrend && sma_20[i-1] > sma_50[i-1]) {  // Just crossed down
                    executeTrade(price_data[i], "SELL", "Trend Reversal", 
                               current_rsi, current_macd, current_signal);
                    continue;
                }
            }
            
            // Record portfolio value
            double portfolio_value = cash + shares * current_price;
            TradingSignal signal;
            signal.date = price_data[i].date;
            signal.action = "HOLD";
            signal.price = current_price;
            signal.rsi = current_rsi;
            signal.macd = current_macd;
            signal.signal = current_signal;
            signal.portfolio_value = portfolio_value;
            signal.reason = "No signal";
            trade_history.push_back(signal);
        }
        
        cout << "\nBacktest completed!" << endl;
    }
    
private:
    void executeTrade(const PriceData& data, const string& action, const string& reason,
                     double rsi, double macd, double signal_val) {
        
        TradingSignal trade_signal;
        trade_signal.date = data.date;
        trade_signal.action = action;
        trade_signal.price = data.close;
        trade_signal.rsi = rsi;
        trade_signal.macd = macd;
        trade_signal.signal = signal_val;
        trade_signal.reason = reason;
        
        if (action == "BUY" && shares == 0 && cash > data.close) {
            double investment = cash * position_size_pct;
            shares = investment / data.close;
            cash -= shares * data.close;
            entry_price = data.close;
            
            cout << "BUY:  " << data.date << " at $" << fixed << setprecision(2) 
                 << data.close << " - " << reason 
                 << " (RSI: " << setprecision(1) << rsi << ")" << endl;
        }
        else if (action == "SELL" && shares > 0) {
            double sale_proceeds = shares * data.close;
            double profit = sale_proceeds - (shares * entry_price);
            cash += sale_proceeds;
            shares = 0;
            
            cout << "SELL: " << data.date << " at $" << fixed << setprecision(2) 
                 << data.close << " - " << reason 
                 << " (Profit: $" << profit << ")" << endl;
            
            entry_price = 0;
        }
        
        trade_signal.portfolio_value = cash + shares * data.close;
        trade_history.push_back(trade_signal);
    }
    
public:
    void printPerformanceMetrics(const vector<PriceData>& price_data) {
        if (trade_history.empty()) {
            cout << "No trading history available." << endl;
            return;
        }
        
        double final_price = price_data.back().close;
        double final_value = cash + shares * final_price;
        double total_return = (final_value - initial_capital) / initial_capital;
        double buy_hold_return = (final_price - price_data[0].close) / price_data[0].close;
        
        // Calculate trade statistics
        int buy_trades = 0, sell_trades = 0, profitable_trades = 0;
        double total_profit = 0.0;
        double last_buy_price = 0.0;
        
        for (const auto& trade : trade_history) {
            if (trade.action == "BUY") {
                buy_trades++;
                last_buy_price = trade.price;
            } else if (trade.action == "SELL" && last_buy_price > 0) {
                sell_trades++;
                double profit = trade.price - last_buy_price;
                total_profit += profit;
                if (profit > 0) profitable_trades++;
            }
        }
        
        double win_rate = buy_trades > 0 ? (double)profitable_trades / buy_trades * 100 : 0;
        
        // Calculate volatility and Sharpe ratio
        vector<double> returns;
        for (size_t i = 1; i < trade_history.size(); ++i) {
            double prev_val = trade_history[i-1].portfolio_value;
            double curr_val = trade_history[i].portfolio_value;
            if (prev_val > 0) {
                returns.push_back((curr_val - prev_val) / prev_val);
            }
        }
        
        double avg_return = 0.0;
        for (double ret : returns) avg_return += ret;
        avg_return /= returns.size();
        
        double variance = 0.0;
        for (double ret : returns) {
            variance += (ret - avg_return) * (ret - avg_return);
        }
        double std_dev = sqrt(variance / returns.size());
        double sharpe = std_dev > 0 ? (avg_return * 252) / (std_dev * sqrt(252)) : 0;
        
        // Maximum drawdown
        double peak = initial_capital;
        double max_drawdown = 0.0;
        for (const auto& trade : trade_history) {
            peak = max(peak, trade.portfolio_value);
            double drawdown = (peak - trade.portfolio_value) / peak;
            max_drawdown = max(max_drawdown, drawdown);
        }
        
        cout << "\n" << string(60, '=') << endl;
        cout << "ADVANCED TRADING STRATEGY RESULTS" << endl;
        cout << string(60, '=') << endl;
        cout << fixed << setprecision(2);
        cout << "Initial Capital:        $" << initial_capital << endl;
        cout << "Final Portfolio:        $" << final_value << endl;
        cout << "Total Return:           " << (total_return * 100) << "%" << endl;
        cout << "Buy & Hold Return:      " << (buy_hold_return * 100) << "%" << endl;
        cout << "Alpha (Excess Return):  " << ((total_return - buy_hold_return) * 100) << "%" << endl;
        cout << "Sharpe Ratio:           " << setprecision(3) << sharpe << endl;
        cout << "Max Drawdown:           " << setprecision(2) << (max_drawdown * 100) << "%" << endl;
        cout << "Total Buy Trades:       " << buy_trades << endl;
        cout << "Total Sell Trades:      " << sell_trades << endl;
        cout << "Win Rate:               " << win_rate << "%" << endl;
        cout << "Current Position:       " << (shares > 0 ? "LONG" : "CASH") << endl;
        cout << "Cash Remaining:         $" << cash << endl;
        if (shares > 0) {
            cout << "Shares Held:            " << setprecision(0) << shares << endl;
            cout << "Unrealized P&L:         $" << setprecision(2) << (shares * (final_price - entry_price)) << endl;
        }

    }
};

int main() {
    try {
        cout << "Advanced Trading Strategy - Multi-Indicator System" << endl;
        cout << string(60, '=') << endl;
        
        // Load price data
        cout << "Loading price data from 'prices.csv'..." << endl;
        vector<PriceData> price_data = CSVParser::loadPriceData("prices.csv");
        cout << "Successfully loaded " << price_data.size() << " price records." << endl;
        
        // Initialize and run strategy
        AdvancedTradingStrategy strategy(100000.0);
        strategy.backtest(price_data);
        strategy.printPerformanceMetrics(price_data);
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        
    }
    
    return 0;
}