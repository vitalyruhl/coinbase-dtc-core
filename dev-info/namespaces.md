# Namespace Documentation

This document contains all namespaces used in the coinbase-dtc-core project for quick reference and consistency.

## Primary Project Namespaces

### Core Namespace
- **`open_dtc_server`** - Main project namespace
  - Location: All project files
  - Purpose: Root namespace for the entire DTC server implementation

### Core Submodules

#### `open_dtc_server::core`
- **Purpose**: Core server functionality and utilities
- **Location**: `include/coinbase_dtc_core/core/` and `src/core/`

#### `open_dtc_server::core::dtc`
- **Purpose**: DTC protocol implementation (messages, serialization, etc.)
- **Location**: `include/coinbase_dtc_core/core/dtc/` and `src/core/dtc/`
- **Key Classes**: `Protocol`, `LogonRequest`, `LogonResponse`, `MarketDataUpdate`, etc.

#### `open_dtc_server::core::server`
- **Purpose**: DTC server implementation (client connections, message handling)
- **Location**: `include/coinbase_dtc_core/core/server/` and `src/core/server/`
- **Key Classes**: `DTCServer`, `ClientConnection`, `ServerConfig`

#### `open_dtc_server::core::util`
- **Purpose**: Utility functions (logging, helpers)
- **Location**: `include/coinbase_dtc_core/core/util/` and `src/core/util/`
- **Key Functions**: `log_debug()`, `log_error()`, `simple_log()`

### Authentication & Credentials

#### `open_dtc_server::auth`
- **Purpose**: Authentication and credential management
- **Location**: `include/coinbase_dtc_core/core/auth/` and `src/core/auth/`
- **Key Classes**: `CDPCredentials`, `JWTAuthenticator`

#### `coinbase_dtc_core::credentials`
- **Purpose**: Legacy credential management (backwards compatibility)
- **Location**: `include/coinbase_dtc_core/core/auth/`
- **Key Classes**: `CredentialsManager`, `CDPCredentialsManager`

### Exchange Integration

#### `open_dtc_server::exchanges`
- **Purpose**: Exchange integration framework
- **Location**: `include/coinbase_dtc_core/exchanges/` and `src/exchanges/`

#### `open_dtc_server::exchanges::base`
- **Purpose**: Base classes and interfaces for exchange implementations
- **Location**: `include/coinbase_dtc_core/exchanges/base/` and `src/exchanges/base/`
- **Key Classes**: `ExchangeFeed`, `MarketTrade`, `MarketLevel2`, `ExchangeConfig`

#### `open_dtc_server::exchanges::coinbase`
- **Purpose**: Coinbase-specific implementation
- **Location**: `include/coinbase_dtc_core/exchanges/coinbase/` and `src/exchanges/coinbase/`
- **Key Classes**: `CoinbaseFeed`, `CoinbaseRestClient`, `WebSocketClient`, `AccountBalance`

#### `open_dtc_server::exchanges::binance`
- **Purpose**: Binance-specific implementation (future)
- **Location**: `include/coinbase_dtc_core/exchanges/binance/` and `src/exchanges/binance/`
- **Status**: Placeholder for future implementation

#### `open_dtc_server::exchanges::factory`
- **Purpose**: Factory pattern for creating exchange implementations
- **Location**: `include/coinbase_dtc_core/exchanges/factory/` and `src/exchanges/factory/`
- **Key Classes**: `ExchangeFactory`

### Test Clients

#### `dtc_test_client`
- **Purpose**: Test client implementations (console and GUI)
- **Location**: `include/dtc_test_client/` and `src/dtc_test_client/`
- **Key Classes**: `DTCTestClient`, `DTCTestClientGUI`

## Namespace Conventions

### Naming Rules
1. **All lowercase** with underscores for separation
2. **Descriptive names** that clearly indicate purpose
3. **Hierarchical structure** following directory structure
4. **Consistent prefixes**: 
   - `open_dtc_server` for core functionality
   - `coinbase_dtc_core` for legacy/compatibility

### Directory-to-Namespace Mapping
```
include/coinbase_dtc_core/
├── core/                     → open_dtc_server::core
│   ├── dtc/                 → open_dtc_server::core::dtc
│   ├── server/              → open_dtc_server::core::server
│   ├── util/                → open_dtc_server::core::util
│   └── auth/                → open_dtc_server::auth
├── exchanges/               → open_dtc_server::exchanges
│   ├── base/               → open_dtc_server::exchanges::base
│   ├── coinbase/           → open_dtc_server::exchanges::coinbase
│   ├── binance/            → open_dtc_server::exchanges::binance
│   └── factory/            → open_dtc_server::exchanges::factory
└── dtc_test_client/        → dtc_test_client
```

## Common Namespace Issues & Solutions

### Mixed Namespaces
- **Problem**: Using `coinbase_dtc_core::credentials` instead of `open_dtc_server::auth`
- **Solution**: Always use the `open_dtc_server` hierarchy for new code

### Forward Declarations
- Use full namespace paths in forward declarations
- Example: `namespace open_dtc_server::core::dtc { class Protocol; }`

### Using Statements
- Prefer explicit namespace qualification over `using namespace`
- Use `using` declarations sparingly and only in implementation files

## Copilot Instructions Addition

**⚠️ NAMESPACE DOCUMENTATION RULE:**

When creating any new namespace in this project:

1. **IMMEDIATELY document it** in `/dev-info/namespaces.md`
2. **Include**:
   - Full namespace path
   - Purpose/responsibility  
   - Location (directories)
   - Key classes/functions
   - Directory-to-namespace mapping
3. **Follow naming conventions**:
   - All lowercase with underscores
   - Use `open_dtc_server` hierarchy
   - Match directory structure
4. **Update this file** before committing any code with new namespaces
5. **Check for conflicts** with existing namespaces before creating new ones

**Example documentation format:**
```markdown
#### `open_dtc_server::new::module`
- **Purpose**: Description of what this namespace contains
- **Location**: `include/path/` and `src/path/`
- **Key Classes**: `ClassName1`, `ClassName2`
- **Status**: Active/Planned/Deprecated
```

**NO EXCEPTIONS** - Every namespace must be documented here for maintainability and consistency.

## Last Updated
- **Date**: November 16, 2025
- **Reason**: Initial namespace documentation creation
- **Changed By**: GitHub Copilot

---

*This document should be updated whenever new namespaces are added or existing ones are modified.*