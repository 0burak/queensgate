#include <vector>
#include <deque>
#include <fstream>
#include <sstream>
#include <string>
#include <numeric>
#include <cmath>
#include <iostream>

using namespace std;

vector<double> gs_prices;
vector<double> ms_prices;

std::vector<double> readCSV(const std::string& filename) {
  std::vector<double> prices;
  std::ifstream file(filename);
  std::string line;

  std::getline(file, line);  // Skip the header line

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string value;
    std::vector<std::string> row;

    while (std::getline(ss, value, ',')) {  // Split the line by commas
      row.push_back(value);
    }

    double adjClose = std::stod(row[5]);  // Assuming "Adj Close" is at index 5
    prices.push_back(adjClose);
  }

  return prices;
}

int main() {
  std::string gs_file = "GS.csv";
  std::string ms_file = "MS.csv";

  gs_prices = readCSV(gs_file);
  ms_prices = readCSV(ms_file);

  const size_t N = 10;

  std::deque<double> spread;

  double cash = 100000;
  double gs_shares = 0;
  double ms_shares = 0;
  int num_shares = 1000;  // The number of shares to long or short
  
  int gs_position = 0; // positive for long, negative for short
  int ms_position = 0; // positive for long, negative for short
  
  int positions_closed = 0;

  for (size_t i = N; i < gs_prices.size(); ++i) {
    spread.push_back(gs_prices[i] - ms_prices[i]);

    if (spread.size() > N) {
      spread.pop_front();
    }

    double mean = std::accumulate(spread.begin(), spread.end(), 0.0) / spread.size();
    double sq_sum = std::inner_product(spread.begin(), spread.end(), spread.begin(), 0.0);
    double stddev = std::sqrt(sq_sum / spread.size() - mean * mean);

    double current_spread = gs_prices[i] - ms_prices[i];
    double z_score = (current_spread - mean) / stddev;

       if (z_score > 1.0) {
      if (gs_shares == 0 && ms_shares == 0) {
        gs_shares += num_shares;
        ms_shares -= num_shares;
        cash -= gs_prices[i] * num_shares;
        cash += ms_prices[i] * num_shares;
        cout << "Day " << i << ": Long GS at price " << gs_prices[i] << ", Short MS at price " << ms_prices[i] << endl;
        gs_position = 1;
        ms_position = -1;
      } else {
        cout << "Day " << i << ": Positions already open, waiting to close." << endl;
      }
    } else if (z_score < -1.0) {
      if (gs_shares == 0 && ms_shares == 0) {
        gs_shares -= num_shares;
        ms_shares += num_shares;
        cash += gs_prices[i] * num_shares;
        cash -= ms_prices[i] * num_shares;
        cout << "Day " << i << ": Short GS at price " << gs_prices[i] << ", Long MS at price " << ms_prices[i] << endl;
        gs_position = -1;
        ms_position = 1;
      } else {
        cout << "Day " << i << ": Positions already open, waiting to close." << endl;
      }
    } else if (abs(z_score) < 0.8) {
      if (gs_position == 1 && ms_position == -1) {
        cash += gs_prices[i] * gs_shares;
        cash -= ms_prices[i] * abs(ms_shares);
        gs_shares = 0;
        ms_shares = 0;
        cout << "Day " << i << ": Closing positions. GS at price " << gs_prices[i] << ", MS at price " << ms_prices[i] << endl;
        positions_closed++;
        gs_position = 0;
        ms_position = 0;
      } else if (gs_position == -1 && ms_position == 1) {
        cash -= gs_prices[i] * abs(gs_shares);
        cash += ms_prices[i] * ms_shares;
        gs_shares = 0;
        ms_shares = 0;
        cout << "Day " << i << ": Closing positions. GS at price " << gs_prices[i] << ", MS at price " << ms_prices[i] << endl;
        positions_closed++;
        gs_position = 0;
        ms_position = 0;
      } else {
        cout << "Day " << i << ": No positions open, waiting to buy." << endl;
      }
    }

    // Print the current cash and portfolio value
    cout << "Day " << i << ": Cash: " << cash << ", Portfolio value: " << cash + (gs_shares * gs_prices[i]) + (ms_shares * ms_prices[i]) << endl;
  }

  double portfolio_value = cash + (gs_shares * gs_prices.back()) + (ms_shares * ms_prices.back());
  cout << "Portfolio value: " << portfolio_value << endl;
  cout << "Positions closed: " << positions_closed << endl;

  return 0;
}
