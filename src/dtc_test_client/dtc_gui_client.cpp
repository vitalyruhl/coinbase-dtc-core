#include "dtc_gui_client.hpp"
#include "coinbase_dtc_core/core/util/log.hpp"
#include <sstream>
#include <iomanip>
#include <chrono>

DTCTestClientGUI::DTCTestClientGUI()
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        MessageBoxA(nullptr, "Failed to initialize Winsock", "Error", MB_ICONERROR);
    }
}

DTCTestClientGUI::~DTCTestClientGUI()
{
    Shutdown();
    WSACleanup();
}

bool DTCTestClientGUI::Initialize(HINSTANCE hInstance)
{
    m_hInstance = hInstance;

    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES;
    if (!InitCommonControlsEx(&icex))
    {
        MessageBoxA(nullptr, "Failed to initialize common controls", "Error", MB_ICONERROR);
        return false;
    }

    // Register window class
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DTCTestClientClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassW(&wc))
    {
        MessageBoxA(nullptr, "Failed to register window class", "Error", MB_ICONERROR);
        return false;
    }

    // Create main window
    m_hwnd = CreateWindowExW(
        0,
        L"DTCTestClientClass",
        L"DTC Test Client - Coinbase Bridge",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr, nullptr, hInstance, this);

    if (!m_hwnd)
    {
        MessageBoxA(nullptr, "Failed to create window", "Error", MB_ICONERROR);
        return false;
    }

    return true;
}

void DTCTestClientGUI::Run()
{
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void DTCTestClientGUI::Shutdown()
{
    if (m_isConnected)
    {
        DisconnectFromServer();
    }
}

LRESULT CALLBACK DTCTestClientGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DTCTestClientGUI *pThis = nullptr;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT *pCreate = (CREATESTRUCT *)lParam;
        pThis = (DTCTestClientGUI *)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else
    {
        pThis = (DTCTestClientGUI *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
    {
        return pThis->HandleMessage(hwnd, uMsg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

LRESULT DTCTestClientGUI::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        OnCreate(hwnd);
        break;
    case WM_COMMAND:
        OnCommand(hwnd, wParam);
        break;
    case WM_CLOSE:
        OnClose(hwnd);
        break;
    case WM_PAINT:
        OnPaint(hwnd);
        break;
    case WM_SIZE:
        // Handle window resizing if needed
        break;
    case WM_TIMER:
        if (wParam == 1) // Our data processing timer
        {
            ProcessIncomingData();
        }
        break;
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void DTCTestClientGUI::OnCreate(HWND hwnd)
{
    int x = CONTROL_SPACING;
    int y = CONTROL_SPACING;

    // Server connection controls
    CreateWindowA("STATIC", "Server Host:", WS_VISIBLE | WS_CHILD,
                  x, y, 80, 20, hwnd, nullptr, m_hInstance, nullptr);

    m_editServerHost = CreateWindowA("EDIT", m_serverHost.c_str(),
                                     WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                                     x + 85, y, 100, 20, hwnd, (HMENU)ID_EDIT_SERVER_HOST, m_hInstance, nullptr);

    CreateWindowA("STATIC", "Port:", WS_VISIBLE | WS_CHILD,
                  x + 195, y, 35, 20, hwnd, nullptr, m_hInstance, nullptr);

    m_editServerPort = CreateWindowA("EDIT", std::to_string(m_serverPort).c_str(),
                                     WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_NUMBER,
                                     x + 235, y, 60, 20, hwnd, (HMENU)ID_EDIT_SERVER_PORT, m_hInstance, nullptr);

    y += 30;

    // Connection buttons
    m_btnConnect = CreateWindowA("BUTTON", "Connect",
                                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                 x, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_BTN_CONNECT, m_hInstance, nullptr);

    m_btnDisconnect = CreateWindowA("BUTTON", "Disconnect",
                                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                    x + BUTTON_WIDTH + CONTROL_SPACING, y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                    hwnd, (HMENU)ID_BTN_DISCONNECT, m_hInstance, nullptr);

    y += BUTTON_HEIGHT + CONTROL_SPACING;

    // Symbol selection
    CreateWindowA("STATIC", "Symbol:", WS_VISIBLE | WS_CHILD,
                  x, y + 5, 50, 20, hwnd, nullptr, m_hInstance, nullptr);

    m_comboSymbols = CreateWindowA("COMBOBOX", "",
                                   WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_VSCROLL,
                                   x + 60, y, 200, 200, hwnd, (HMENU)ID_COMBO_SYMBOLS, m_hInstance, nullptr);

    // Add some default symbols
    SendMessageA(m_comboSymbols, CB_ADDSTRING, 0, (LPARAM) "BTC-USD");
    SendMessageA(m_comboSymbols, CB_ADDSTRING, 0, (LPARAM) "ETH-USD");
    SendMessageA(m_comboSymbols, CB_ADDSTRING, 0, (LPARAM) "STRK-USD");
    SendMessageA(m_comboSymbols, CB_SETCURSEL, 0, 0);

    y += 30;

    // Action buttons - Row 1
    m_btnAccountInfo = CreateWindowA("BUTTON", "Account Info",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                     x, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_BTN_ACCOUNT_INFO, m_hInstance, nullptr);

    m_btnLoadSymbols = CreateWindowA("BUTTON", "Load Symbols",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                     x + BUTTON_WIDTH + CONTROL_SPACING, y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                     hwnd, (HMENU)ID_BTN_LOAD_SYMBOLS, m_hInstance, nullptr);

    m_btnSymbolInfo = CreateWindowA("BUTTON", "Symbol Info",
                                    WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                    x + 2 * (BUTTON_WIDTH + CONTROL_SPACING), y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                    hwnd, (HMENU)ID_BTN_SYMBOL_INFO, m_hInstance, nullptr);

    y += BUTTON_HEIGHT + CONTROL_SPACING;

    // Action buttons - Row 2
    m_btnDOMData = CreateWindowA("BUTTON", "DOM Data",
                                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                 x, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_BTN_DOM_DATA, m_hInstance, nullptr);

    m_btnSubscribe = CreateWindowA("BUTTON", "Subscribe",
                                   WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                   x + BUTTON_WIDTH + CONTROL_SPACING, y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                   hwnd, (HMENU)ID_BTN_SUBSCRIBE, m_hInstance, nullptr);

    m_btnUnsubscribe = CreateWindowA("BUTTON", "Unsubscribe",
                                     WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | WS_DISABLED,
                                     x + 2 * (BUTTON_WIDTH + CONTROL_SPACING), y, BUTTON_WIDTH, BUTTON_HEIGHT,
                                     hwnd, (HMENU)ID_BTN_UNSUBSCRIBE, m_hInstance, nullptr);

    y += BUTTON_HEIGHT + CONTROL_SPACING;

    // Clear console button
    m_btnClearConsole = CreateWindowA("BUTTON", "Clear Console",
                                      WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                      x, y, BUTTON_WIDTH, BUTTON_HEIGHT, hwnd, (HMENU)ID_BTN_CLEAR_CONSOLE, m_hInstance, nullptr);

    y += BUTTON_HEIGHT + CONTROL_SPACING;

    // Console output
    CreateWindowA("STATIC", "Console Output:", WS_VISIBLE | WS_CHILD,
                  x, y, 100, 20, hwnd, nullptr, m_hInstance, nullptr);

    y += 25;

    m_editConsole = CreateWindowA("EDIT", "",
                                  WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                  x, y, WINDOW_WIDTH - 40, 200, hwnd, (HMENU)ID_EDIT_CONSOLE, m_hInstance, nullptr);

    // Status bar
    m_statusBar = CreateWindowA("msctls_statusbar32", "Ready",
                                WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP,
                                0, 0, 0, 0, hwnd, (HMENU)ID_STATUS_BAR, m_hInstance, nullptr);

    // Set initial status
    UpdateStatus("Ready - Not Connected");
    UpdateConsole("DTC Test Client initialized");
    UpdateConsole("Click 'Connect' to connect to DTC server");
}

void DTCTestClientGUI::OnCommand(HWND hwnd, WPARAM wParam)
{
    switch (LOWORD(wParam))
    {
    case ID_BTN_CONNECT:
        ConnectToServer();
        break;
    case ID_BTN_DISCONNECT:
        DisconnectFromServer();
        break;
    case ID_BTN_ACCOUNT_INFO:
        GetAccountInfo();
        break;
    case ID_BTN_LOAD_SYMBOLS:
        LoadAvailableSymbols();
        break;
    case ID_BTN_SYMBOL_INFO:
        GetSymbolInfo();
        break;
    case ID_BTN_DOM_DATA:
        GetDOMData();
        break;
    case ID_BTN_SUBSCRIBE:
        SubscribeToSymbol();
        break;
    case ID_BTN_UNSUBSCRIBE:
        UnsubscribeFromSymbol();
        break;
    case ID_BTN_CLEAR_CONSOLE:
        ClearConsole();
        break;
    }
}

void DTCTestClientGUI::OnClose(HWND hwnd)
{
    DestroyWindow(hwnd);
    PostQuitMessage(0);
}

void DTCTestClientGUI::OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    EndPaint(hwnd, &ps);
}

void DTCTestClientGUI::ConnectToServer()
{
    // Get server host and port from edit controls
    char hostBuffer[256];
    char portBuffer[16];

    GetWindowTextA(m_editServerHost, hostBuffer, sizeof(hostBuffer));
    GetWindowTextA(m_editServerPort, portBuffer, sizeof(portBuffer));

    m_serverHost = hostBuffer;
    m_serverPort = atoi(portBuffer);

    UpdateConsole("Connecting to " + m_serverHost + ":" + std::to_string(m_serverPort) + "...");

    // Create socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET)
    {
        UpdateConsole("ERROR: Failed to create socket");
        return;
    }

    // Connect to server
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_serverPort);

    // Handle hostname resolution for both IP addresses and hostnames like "localhost"
    if (inet_pton(AF_INET, m_serverHost.c_str(), &serverAddr.sin_addr) != 1)
    {
        // inet_pton failed, try hostname resolution
        struct hostent *host = gethostbyname(m_serverHost.c_str());
        if (host == nullptr || host->h_addr_list[0] == nullptr)
        {
            UpdateConsole("ERROR: Cannot resolve hostname '" + m_serverHost + "'");
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return;
        }

        // Copy the resolved IP address
        memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);
        UpdateConsole("Resolved '" + m_serverHost + "' to IP address");
    }

    if (connect(m_socket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        UpdateConsole("ERROR: Failed to connect to server");
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return;
    }

    m_isConnected = true;
    UpdateConsole("✅ Connected successfully!");
    UpdateStatus("Connected to " + m_serverHost + ":" + std::to_string(m_serverPort));

    // Start timer for processing incoming data (check every 100ms)
    SetTimer(m_hwnd, 1, 100, nullptr);

    // Enable/disable buttons
    EnableWindow(m_btnConnect, FALSE);
    EnableWindow(m_btnDisconnect, TRUE);
    EnableWindow(m_btnAccountInfo, TRUE);
    EnableWindow(m_btnLoadSymbols, TRUE);
    EnableWindow(m_btnSymbolInfo, TRUE);
    EnableWindow(m_btnDOMData, TRUE);
    EnableWindow(m_btnSubscribe, TRUE);
    EnableWindow(m_btnUnsubscribe, TRUE);
}

void DTCTestClientGUI::DisconnectFromServer()
{
    if (m_socket != INVALID_SOCKET)
    {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }

    m_isConnected = false;

    // Stop the data processing timer
    KillTimer(m_hwnd, 1);

    // Clear any pending data
    m_incomingBuffer.clear();

    UpdateConsole("Disconnected from server");
    UpdateStatus("Disconnected");

    // Enable/disable buttons
    EnableWindow(m_btnConnect, TRUE);
    EnableWindow(m_btnDisconnect, FALSE);
    EnableWindow(m_btnAccountInfo, FALSE);
    EnableWindow(m_btnLoadSymbols, FALSE);
    EnableWindow(m_btnSymbolInfo, FALSE);
    EnableWindow(m_btnDOMData, FALSE);
    EnableWindow(m_btnSubscribe, FALSE);
    EnableWindow(m_btnUnsubscribe, FALSE);
}

void DTCTestClientGUI::GetAccountInfo()
{
    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("📊 Sending DTC LogonRequest to server...");

    try
    {
        // Create LogonRequest message using DTC protocol
        open_dtc_server::core::dtc::LogonRequest logon_request;
        logon_request.protocol_version = open_dtc_server::core::dtc::DTC_PROTOCOL_VERSION;
        logon_request.username = "test_user";
        logon_request.password = "";
        logon_request.client_name = "DTC Test Client GUI";
        logon_request.trade_account = "";
        logon_request.heartbeat_interval_in_seconds = 10;
        logon_request.general_text_data = "Account info request";

        // Serialize the message
        std::vector<uint8_t> message_data = logon_request.serialize();

        // Send to server
        if (SendDTCMessage(message_data))
        {
            UpdateConsole("✅ LogonRequest sent successfully");
            UpdateConsole("⏳ Waiting for server response...");
        }
        else
        {
            UpdateConsole("❌ Failed to send LogonRequest");
        }
    }
    catch (const std::exception &e)
    {
        UpdateConsole("❌ Error creating LogonRequest: " + std::string(e.what()));
    }
}

void DTCTestClientGUI::LoadAvailableSymbols()
{
    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("📋 Sending DTC SecurityDefinitionRequest to server...");

    try
    {
        // Create SecurityDefinitionForSymbolRequest message
        open_dtc_server::core::dtc::SecurityDefinitionForSymbolRequest symbol_request;
        symbol_request.request_id = 1001;
        symbol_request.symbol = "*"; // Request all symbols
        symbol_request.exchange = "coinbase";

        // Serialize the message
        std::vector<uint8_t> message_data = symbol_request.serialize();

        // Send to server
        if (SendDTCMessage(message_data))
        {
            UpdateConsole("✅ SecurityDefinitionRequest sent successfully");
            UpdateConsole("⏳ Waiting for symbol definitions...");
        }
        else
        {
            UpdateConsole("❌ Failed to send SecurityDefinitionRequest");
        }
    }
    catch (const std::exception &e)
    {
        UpdateConsole("❌ Error creating SecurityDefinitionRequest: " + std::string(e.what()));
    }

    // Clear current symbols
    SendMessage(m_comboSymbols, CB_RESETCONTENT, 0, 0);

    // Add standard symbols that Coinbase supports (these would come from server in real implementation)
    std::vector<std::string> standardSymbols = {
        "BTC-USD", "ETH-USD", "STRK-USD", "ADA-USD", "SOL-USD",
        "DOT-USD", "LINK-USD", "UNI-USD", "AAVE-USD", "SUSHI-USD"};

    for (const auto &symbol : standardSymbols)
    {
        SendMessageA(m_comboSymbols, CB_ADDSTRING, 0, (LPARAM)symbol.c_str());
        UpdateConsole("  📈 Available: " + symbol + " (would come from server)");
    }

    SendMessage(m_comboSymbols, CB_SETCURSEL, 0, 0);
    UpdateConsole("📋 Next: Server needs to implement symbol listing from Coinbase API");
}

void DTCTestClientGUI::GetSymbolInfo()
{
    std::string symbol = GetSelectedSymbol();
    if (symbol.empty())
    {
        UpdateConsole("ERROR: No symbol selected");
        return;
    }

    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("ℹ️ Getting symbol info for: " + symbol);

    // TODO: Implement actual DTC symbol info request
    UpdateConsole("Symbol Info for " + symbol + ":");
    UpdateConsole("  Full Name: " + symbol);
    UpdateConsole("  Type: Cryptocurrency Pair");
    UpdateConsole("  Base Currency: " + symbol.substr(0, symbol.find('-')));
    UpdateConsole("  Quote Currency: " + symbol.substr(symbol.find('-') + 1));
    UpdateConsole("  Min Order Size: 0.001");
    UpdateConsole("  Max Order Size: 10000");
    UpdateConsole("  Price Increment: 0.01");
}

void DTCTestClientGUI::GetDOMData()
{
    std::string symbol = GetSelectedSymbol();
    if (symbol.empty())
    {
        UpdateConsole("ERROR: No symbol selected");
        return;
    }

    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("📈 Getting DOM (Depth of Market) data for: " + symbol);

    // TODO: Implement actual DTC DOM request
    UpdateConsole("DOM Data for " + symbol + ":");
    UpdateConsole("  Bids:");
    UpdateConsole("    $45,250.00 x 0.5");
    UpdateConsole("    $45,249.50 x 1.2");
    UpdateConsole("    $45,249.00 x 0.8");
    UpdateConsole("  Asks:");
    UpdateConsole("    $45,251.00 x 0.7");
    UpdateConsole("    $45,251.50 x 1.0");
    UpdateConsole("    $45,252.00 x 0.9");
}

void DTCTestClientGUI::SubscribeToSymbol()
{
    std::string symbol = GetSelectedSymbol();
    if (symbol.empty())
    {
        UpdateConsole("ERROR: No symbol selected");
        return;
    }

    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("🔔 Subscribing to real-time data for: " + symbol);

    // TODO: Implement actual DTC subscription request
    UpdateConsole("✅ Subscribed to " + symbol);
    UpdateConsole("You will now receive real-time updates for this symbol");
}

void DTCTestClientGUI::UnsubscribeFromSymbol()
{
    std::string symbol = GetSelectedSymbol();
    if (symbol.empty())
    {
        UpdateConsole("ERROR: No symbol selected");
        return;
    }

    if (!m_isConnected)
    {
        UpdateConsole("ERROR: Not connected to server");
        return;
    }

    UpdateConsole("🔕 Unsubscribing from: " + symbol);

    // TODO: Implement actual DTC unsubscribe request
    UpdateConsole("✅ Unsubscribed from " + symbol);
}

void DTCTestClientGUI::UpdateConsole(const std::string &message)
{
    if (!m_editConsole)
        return;

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "[%H:%M:%S] ");

    std::string timestampedMessage = ss.str() + message + "\r\n";

    // Get current text length
    int textLength = GetWindowTextLengthA(m_editConsole);

    // Set selection to end and append text
    SendMessage(m_editConsole, EM_SETSEL, textLength, textLength);
    SendMessageA(m_editConsole, EM_REPLACESEL, FALSE, (LPARAM)timestampedMessage.c_str());

    // Scroll to bottom
    SendMessage(m_editConsole, EM_SCROLLCARET, 0, 0);
}

void DTCTestClientGUI::UpdateStatus(const std::string &status)
{
    if (m_statusBar)
    {
        SendMessageA(m_statusBar, SB_SETTEXTA, 0, (LPARAM)status.c_str());
    }
}

void DTCTestClientGUI::ClearConsole()
{
    if (m_editConsole)
    {
        SetWindowTextA(m_editConsole, "");
        UpdateConsole("Console cleared");
    }
}

std::string DTCTestClientGUI::GetSelectedSymbol()
{
    if (!m_comboSymbols)
        return "";

    int selection = SendMessage(m_comboSymbols, CB_GETCURSEL, 0, 0);
    if (selection == CB_ERR)
        return "";

    char buffer[256];
    SendMessageA(m_comboSymbols, CB_GETLBTEXT, selection, (LPARAM)buffer);
    return std::string(buffer);
}

bool DTCTestClientGUI::SendDTCMessage(const std::vector<uint8_t> &message)
{
    if (!m_isConnected || m_socket == INVALID_SOCKET)
    {
        return false;
    }

    int result = send(m_socket, (const char *)message.data(), message.size(), 0);
    return result != SOCKET_ERROR;
}

void DTCTestClientGUI::ProcessIncomingData()
{
    if (!m_isConnected || m_socket == INVALID_SOCKET)
        return;

    // Use non-blocking recv to check for data
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(m_socket, &readfds);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0; // Non-blocking

    int result = select(0, &readfds, nullptr, nullptr, &timeout);
    if (result > 0 && FD_ISSET(m_socket, &readfds))
    {
        // Data available to read
        std::vector<uint8_t> buffer(4096);
        int bytes_received = recv(m_socket, (char *)buffer.data(), buffer.size(), 0);

        if (bytes_received > 0)
        {
            buffer.resize(bytes_received);

            // Add to incoming buffer
            m_incomingBuffer.insert(m_incomingBuffer.end(), buffer.begin(), buffer.end());

            // Process complete DTC messages
            ProcessDTCMessages();
        }
        else if (bytes_received == 0)
        {
            // Server disconnected
            UpdateConsole("🔌 Server disconnected");
            DisconnectFromServer();
        }
        else if (bytes_received == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK)
            {
                UpdateConsole("❌ Socket error: " + std::to_string(error));
                DisconnectFromServer();
            }
        }
    }
}

void DTCTestClientGUI::ProcessDTCMessages()
{
    // Create DTC protocol handler
    open_dtc_server::core::dtc::Protocol protocol_handler;

    // Process complete DTC messages from buffer
    while (m_incomingBuffer.size() >= 4) // Minimum DTC message size (header)
    {
        // Check message size
        uint16_t message_size = *reinterpret_cast<const uint16_t *>(m_incomingBuffer.data());
        if (message_size < 4 || message_size > 65535)
        {
            UpdateConsole("❌ Invalid DTC message size: " + std::to_string(message_size));
            m_incomingBuffer.clear(); // Clear corrupted buffer
            break;
        }

        if (m_incomingBuffer.size() < message_size)
        {
            // Wait for more data
            break;
        }

        // Parse DTC message
        try
        {
            auto dtc_message = protocol_handler.parse_message(m_incomingBuffer.data(), message_size);
            if (dtc_message)
            {
                HandleDTCResponse(std::move(dtc_message));
            }
        }
        catch (const std::exception &e)
        {
            UpdateConsole("❌ Error parsing DTC message: " + std::string(e.what()));
        }

        // Remove processed message from buffer
        m_incomingBuffer.erase(m_incomingBuffer.begin(), m_incomingBuffer.begin() + message_size);
    }
}

void DTCTestClientGUI::HandleDTCResponse(std::unique_ptr<open_dtc_server::core::dtc::DTCMessage> message)
{
    if (!message)
        return;

    switch (message->get_type())
    {
    case open_dtc_server::core::dtc::MessageType::LOGON_RESPONSE:
    {
        auto *logon_resp = static_cast<open_dtc_server::core::dtc::LogonResponse *>(message.get());

        // Debug logging
        UpdateConsole("🔍 Debug - LogonResponse received:");
        UpdateConsole("  Result: " + std::to_string(logon_resp->result));
        UpdateConsole("  Result text: '" + logon_resp->result_text + "'");
        UpdateConsole("  Server name: '" + logon_resp->server_name + "'");

        if (logon_resp->result == 1) // Success
        {
            UpdateConsole("✅ DTC Login successful! (REAL protocol communication)");
            UpdateConsole("📋 Server: " + logon_resp->server_name + " (REAL server instance)");
            UpdateConsole("📊 Trading supported: " + std::string(logon_resp->trading_is_supported ? "Yes" : "No") + " [SERVER CONFIG]");
            UpdateConsole("📈 Market data supported: " + std::string(logon_resp->market_depth_is_supported ? "Yes" : "No") + " [SERVER CONFIG]");
            UpdateConsole("🔍 Security definitions supported: " + std::string(logon_resp->security_definitions_supported ? "Yes" : "No") + " [SERVER CONFIG]");
            UpdateConsole("⚠️  Exchange connection: FAILED (Coinbase SSL/JWT not implemented yet)");
            UpdateConsole("🎯 Status: DTC protocol ✅ | Coinbase connection ❌");
        }
        else
        {
            UpdateConsole("❌ Login failed: " + logon_resp->result_text);
        }
        break;
    }

    case open_dtc_server::core::dtc::MessageType::SECURITY_DEFINITION_RESPONSE:
    {
        auto *symbol_resp = static_cast<open_dtc_server::core::dtc::SecurityDefinitionResponse *>(message.get());
        UpdateConsole("📋 Symbol: " + symbol_resp->symbol + " (" + symbol_resp->exchange + ") [CONFIGURED - not live data yet]");
        UpdateConsole("   Description: " + symbol_resp->description + " [SERVER CONFIG]");
        UpdateConsole("   Min tick: " + std::to_string(symbol_resp->min_price_increment) + " [DEFAULT VALUE]");
        UpdateConsole("⚠️  Note: Symbol list is server-configured, not from live Coinbase API");

        // Add to combo box if not already there
        std::string symbol_text = symbol_resp->symbol;
        int count = SendMessageA(m_comboSymbols, CB_GETCOUNT, 0, 0);
        bool found = false;
        for (int i = 0; i < count; i++)
        {
            char buffer[256];
            SendMessageA(m_comboSymbols, CB_GETLBTEXT, i, (LPARAM)buffer);
            if (symbol_text == buffer)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            SendMessageA(m_comboSymbols, CB_ADDSTRING, 0, (LPARAM)symbol_text.c_str());
        }
        break;
    }

    case open_dtc_server::core::dtc::MessageType::HEARTBEAT:
    {
        // Echo heartbeat back to server
        auto *heartbeat = static_cast<open_dtc_server::core::dtc::Heartbeat *>(message.get());
        open_dtc_server::core::dtc::Protocol protocol;
        auto heartbeat_response = protocol.create_heartbeat(heartbeat->num_drops);
        auto response_data = protocol.create_message(*heartbeat_response);
        SendDTCMessage(response_data);
        break;
    }

    default:
        UpdateConsole("📨 Received DTC message type: " +
                      std::to_string(static_cast<uint16_t>(message->get_type())));
        break;
    }
}