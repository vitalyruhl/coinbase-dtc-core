#include "coinbase_dtc_core/exchanges/factory/exchange_factory.hpp"
#include "coinbase_dtc_core/exchanges/coinbase/coinbase_feed.hpp"
#include "coinbase_dtc_core/exchanges/binance/binance_feed.hpp"
#include "coinbase_dtc_core/core/util/log.hpp"
#include <stdexcept>
#include <algorithm>

namespace open_dtc_server
{
    namespace exchanges
    {
        namespace factory
        {

            std::unique_ptr<base::ExchangeFeedBase> ExchangeFactory::create_feed(const base::ExchangeConfig &config)
            {
                open_dtc_server::util::simple_log("[FACTORY] Creating feed for exchange: " + config.name);

                // Convert exchange name to lowercase for comparison
                std::string exchange_name = config.name;
                std::transform(exchange_name.begin(), exchange_name.end(), exchange_name.begin(), ::tolower);

                if (exchange_name == "coinbase")
                {
                    auto feed = std::make_unique<coinbase::CoinbaseFeed>(config);
                    return std::move(feed);
                }
                else if (exchange_name == "binance")
                {
                    auto feed = std::make_unique<binance::BinanceFeed>(config);
                    return std::move(feed);
                }
                else
                {
                    throw std::invalid_argument("Unsupported exchange: " + config.name +
                                                ". Supported exchanges: coinbase, binance");
                }
            }

        } // namespace factory
    } // namespace exchanges
} // namespace open_dtc_server
