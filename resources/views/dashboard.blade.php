<!DOCTYPE html>
<html lang="id">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <meta name="theme-color" content="#1a1a2e">
    <meta name="description" content="Gas Monitor - Sistem Pendeteksi Kebocoran Gas LPG">
    <meta name="csrf-token" content="{{ csrf_token() }}">

    <!-- PWA Manifest -->
    <link rel="manifest" href="/manifest.json">
    <link rel="apple-touch-icon" href="/icons/icon-192.png">

    <title>Gas Monitor - SKRIPSI</title>

    <!-- Google Fonts -->
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">

    <!-- Chart.js -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

    <!-- Firebase SDK -->
    <script src="https://www.gstatic.com/firebasejs/9.22.0/firebase-app-compat.js"></script>
    <script src="https://www.gstatic.com/firebasejs/9.22.0/firebase-messaging-compat.js"></script>

    <style>
        :root {
            --bg-primary: #0f0f1a;
            --bg-secondary: #1a1a2e;
            --bg-card: #16213e;
            --accent: #00d9ff;
            --accent-green: #00ff88;
            --accent-red: #ff4757;
            --accent-orange: #ffa502;
            --text-primary: #ffffff;
            --text-secondary: #a0a0a0;
            --shadow: 0 10px 40px rgba(0, 0, 0, 0.3);
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Inter', sans-serif;
            background: var(--bg-primary);
            color: var(--text-primary);
            min-height: 100vh;
            overflow-x: hidden;
        }

        /* Header */
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 20px 24px;
            background: var(--bg-secondary);
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .header-title {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .header-title h1 {
            font-size: 20px;
            font-weight: 600;
        }

        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: var(--accent-red);
            animation: pulse 2s infinite;
        }

        .status-dot.online {
            background: var(--accent-green);
        }

        @keyframes pulse {

            0%,
            100% {
                opacity: 1;
            }

            50% {
                opacity: 0.5;
            }
        }

        .header-icons {
            display: flex;
            gap: 16px;
        }

        .header-icons button {
            background: none;
            border: none;
            color: var(--text-secondary);
            font-size: 20px;
            cursor: pointer;
            transition: color 0.3s;
        }

        .header-icons button:hover {
            color: var(--accent);
        }

        .header-icons button.notif-active {
            color: var(--accent-green);
        }

        /* Main Container */
        .container {
            padding: 24px;
            max-width: 500px;
            margin: 0 auto;
        }

        /* Gauge Section */
        .gauge-section {
            background: var(--bg-card);
            border-radius: 24px;
            padding: 32px 24px;
            margin-bottom: 24px;
            box-shadow: var(--shadow);
            text-align: center;
        }

        .gauge-label {
            color: var(--text-secondary);
            font-size: 14px;
            margin-bottom: 20px;
        }

        .gauge-container {
            position: relative;
            width: 200px;
            height: 120px;
            margin: 0 auto 20px;
        }

        .gauge-svg {
            width: 100%;
            height: 100%;
        }

        .gauge-bg {
            fill: none;
            stroke: rgba(255, 255, 255, 0.1);
            stroke-width: 20;
            stroke-linecap: round;
        }

        .gauge-fill {
            fill: none;
            stroke: var(--accent-green);
            stroke-width: 20;
            stroke-linecap: round;
            stroke-dasharray: 251.2;
            stroke-dashoffset: 251.2;
            transition: stroke-dashoffset 1s ease, stroke 0.5s ease;
        }

        .gauge-fill.warning {
            stroke: var(--accent-orange);
        }

        .gauge-fill.danger {
            stroke: var(--accent-red);
        }

        .gauge-value {
            position: absolute;
            bottom: 0;
            left: 50%;
            transform: translateX(-50%);
            font-size: 48px;
            font-weight: 700;
        }

        .gauge-unit {
            font-size: 16px;
            color: var(--text-secondary);
            font-weight: 400;
        }

        .gauge-scale {
            display: flex;
            justify-content: space-between;
            padding: 0 10px;
            color: var(--text-secondary);
            font-size: 12px;
        }

        /* Controls Section */
        .controls-section {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-bottom: 24px;
        }

        .control-card {
            background: var(--bg-card);
            border-radius: 20px;
            padding: 20px 16px;
            box-shadow: var(--shadow);
        }

        .control-label {
            color: var(--text-secondary);
            font-size: 13px;
            margin-bottom: 12px;
            text-align: center;
        }

        /* Toggle Switch */
        .toggle-container {
            display: flex;
            gap: 6px;
        }

        .toggle-btn {
            flex: 1;
            padding: 12px 6px;
            border: none;
            border-radius: 12px;
            font-size: 12px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 4px;
            white-space: nowrap;
        }

        .toggle-btn.active {
            background: linear-gradient(135deg, var(--accent), #00a8cc);
            color: white;
            box-shadow: 0 4px 20px rgba(0, 217, 255, 0.3);
        }

        .toggle-btn:not(.active) {
            background: rgba(255, 255, 255, 0.05);
            color: var(--text-secondary);
        }

        .toggle-btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
        }

        /* Status indicator in toggle */
        .toggle-icon {
            font-size: 14px;
        }

        /* Chart Section */
        .chart-section {
            background: var(--bg-card);
            border-radius: 24px;
            padding: 24px;
            box-shadow: var(--shadow);
            margin-bottom: 24px;
        }

        .chart-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 20px;
        }

        .chart-title {
            font-size: 15px;
            font-weight: 500;
        }

        .chart-period {
            display: flex;
            gap: 4px;
        }

        .period-btn {
            padding: 6px 12px;
            border: none;
            border-radius: 8px;
            font-size: 12px;
            cursor: pointer;
            background: rgba(255, 255, 255, 0.05);
            color: var(--text-secondary);
            transition: all 0.3s;
        }

        .period-btn.active {
            background: var(--accent);
            color: var(--bg-primary);
        }

        .chart-container {
            height: 180px;
        }

        .no-data {
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100%;
            color: var(--text-secondary);
            font-size: 14px;
        }

        /* Alert Banner */
        .alert-banner {
            display: none;
            background: linear-gradient(135deg, var(--accent-red), #ff6b6b);
            border-radius: 16px;
            padding: 20px;
            margin-bottom: 24px;
            animation: alertPulse 1s infinite;
        }

        .alert-banner.show {
            display: block;
        }

        @keyframes alertPulse {

            0%,
            100% {
                transform: scale(1);
            }

            50% {
                transform: scale(1.02);
            }
        }

        .alert-content {
            display: flex;
            align-items: center;
            gap: 12px;
        }

        .alert-icon {
            font-size: 32px;
        }

        .alert-text h3 {
            font-size: 16px;
            margin-bottom: 4px;
        }

        .alert-text p {
            font-size: 13px;
            opacity: 0.9;
        }

        /* Connection Status */
        .connection-status {
            text-align: center;
            padding: 12px;
            font-size: 12px;
            color: var(--text-secondary);
        }

        /* Install PWA Button */
        .install-btn {
            display: none;
            width: 100%;
            padding: 16px;
            background: linear-gradient(135deg, var(--accent), #00a8cc);
            border: none;
            border-radius: 16px;
            color: white;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            margin-bottom: 24px;
        }

        .install-btn.show {
            display: block;
        }

        /* Notification Banner */
        .notif-banner {
            display: none;
            background: linear-gradient(135deg, #4CAF50, #45a049);
            border-radius: 16px;
            padding: 16px 20px;
            margin-bottom: 24px;
            cursor: pointer;
        }

        .notif-banner.show {
            display: block;
        }

        .notif-banner p {
            font-size: 14px;
            text-align: center;
        }

        /* Loading Overlay */
        .loading-overlay {
            position: fixed;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background: rgba(0, 0, 0, 0.7);
            display: none;
            align-items: center;
            justify-content: center;
            z-index: 1000;
        }

        .loading-overlay.show {
            display: flex;
        }

        .spinner {
            width: 50px;
            height: 50px;
            border: 4px solid rgba(255, 255, 255, 0.1);
            border-top-color: var(--accent);
            border-radius: 50%;
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            to {
                transform: rotate(360deg);
            }
        }

        /* Toast Notification */
        .toast {
            position: fixed;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%);
            background: var(--bg-card);
            color: white;
            padding: 12px 24px;
            border-radius: 12px;
            font-size: 14px;
            z-index: 1001;
            display: none;
            box-shadow: 0 4px 20px rgba(0,0,0,0.3);
        }

        .toast.show {
            display: block;
            animation: slideUp 0.3s ease;
        }

        @keyframes slideUp {
            from {
                transform: translateX(-50%) translateY(20px);
                opacity: 0;
            }
            to {
                transform: translateX(-50%) translateY(0);
                opacity: 1;
            }
        }
    </style>
</head>

<body>
    <!-- Header -->
    <header class="header">
        <div class="header-title">
            <h1>GAS-MONITOR</h1>
            <div class="status-dot" id="statusDot"></div>
        </div>
        <div class="header-icons">
            <button id="refreshBtn" title="Refresh">🔄</button>
            <button id="notifBtn" title="Aktifkan Notifikasi">🔔</button>
            <button id="infoBtn" title="Info">ℹ️</button>
        </div>
    </header>

    <!-- Main Container -->
    <main class="container">
        <!-- Alert Banner -->
        <div class="alert-banner" id="alertBanner">
            <div class="alert-content">
                <span class="alert-icon">⚠️</span>
                <div class="alert-text">
                    <h3>Kebocoran Gas Terdeteksi!</h3>
                    <p>Sistem keamanan telah diaktifkan secara otomatis.</p>
                </div>
            </div>
        </div>

        <!-- Notification Enable Banner -->
        <div class="notif-banner" id="notifBanner">
            <p>🔔 Klik di sini untuk aktifkan notifikasi push!</p>
        </div>

        <!-- Install PWA Button -->
        <button class="install-btn" id="installBtn">📱 Install Aplikasi</button>

        <!-- Gauge Section -->
        <section class="gauge-section">
            <div class="gauge-label">Kadar Gas</div>
            <div class="gauge-container">
                <svg class="gauge-svg" viewBox="0 0 200 120">
                    <path class="gauge-bg" d="M 20,100 A 80,80 0 0,1 180,100"></path>
                    <path class="gauge-fill" id="gaugeFill" d="M 20,100 A 80,80 0 0,1 180,100"></path>
                </svg>
                <div class="gauge-value">
                    <span id="gasValue">0</span><span class="gauge-unit">%</span>
                </div>
            </div>
            <div class="gauge-scale">
                <span>0</span>
                <span>100</span>
            </div>
        </section>

        <!-- Controls Section -->
        <section class="controls-section">
            <div class="control-card">
                <div class="control-label">Mode</div>
                <div class="toggle-container">
                    <button class="toggle-btn active" id="modeAuto" data-mode="auto">
                        <span class="toggle-icon">🤖</span> Auto
                    </button>
                    <button class="toggle-btn" id="modeManual" data-mode="manual">
                        <span class="toggle-icon">👆</span> Manual
                    </button>
                </div>
            </div>
            <div class="control-card">
                <div class="control-label">Komponen</div>
                <div class="toggle-container">
                    <button class="toggle-btn active" id="valveOn" data-status="on">
                        <span class="toggle-icon">✓</span> ON
                    </button>
                    <button class="toggle-btn" id="valveOff" data-status="off">
                        <span class="toggle-icon">✕</span> OFF
                    </button>
                </div>
            </div>
        </section>

        <!-- Chart Section -->
        <section class="chart-section">
            <div class="chart-header">
                <span class="chart-title">Data Kadar Gas</span>
                <div class="chart-period">
                    <button class="period-btn active" data-hours="6">6H</button>
                    <button class="period-btn" data-hours="12">12H</button>
                    <button class="period-btn" data-hours="24">1D</button>
                </div>
            </div>
            <div class="chart-container">
                <canvas id="gasChart"></canvas>
                <div class="no-data" id="noData">No data yet...</div>
            </div>
        </section>

        <!-- Connection Status -->
        <div class="connection-status" id="connectionStatus">
            Memuat data...
        </div>
    </main>

    <!-- Loading Overlay -->
    <div class="loading-overlay" id="loadingOverlay">
        <div class="spinner"></div>
    </div>

    <!-- Toast -->
    <div class="toast" id="toast"></div>

    <script>
        // Configuration
        const POLLING_INTERVAL = 1000; // 1 second
        const API_BASE = '';
        
        // Firebase Configuration - GANTI DENGAN CONFIG ANDA
        const firebaseConfig = {
            apiKey: "YOUR_API_KEY",
            authDomain: "YOUR_PROJECT.firebaseapp.com",
            projectId: "YOUR_PROJECT_ID",
            storageBucket: "YOUR_PROJECT.appspot.com",
            messagingSenderId: "YOUR_SENDER_ID",
            appId: "YOUR_APP_ID"
        };

        // State
        let chart = null;
        let currentPeriod = 6;
        let isOnline = false;
        let deferredPrompt = null;
        let messaging = null;
        let notificationEnabled = false;

        // DOM Elements
        const statusDot = document.getElementById('statusDot');
        const gasValue = document.getElementById('gasValue');
        const gaugeFill = document.getElementById('gaugeFill');
        const alertBanner = document.getElementById('alertBanner');
        const modeAuto = document.getElementById('modeAuto');
        const modeManual = document.getElementById('modeManual');
        const valveOn = document.getElementById('valveOn');
        const valveOff = document.getElementById('valveOff');
        const connectionStatus = document.getElementById('connectionStatus');
        const loadingOverlay = document.getElementById('loadingOverlay');
        const installBtn = document.getElementById('installBtn');
        const noData = document.getElementById('noData');
        const notifBtn = document.getElementById('notifBtn');
        const notifBanner = document.getElementById('notifBanner');
        const toast = document.getElementById('toast');

        // CSRF Token
        const csrfToken = document.querySelector('meta[name="csrf-token"]').content;

        // Show Toast
        function showToast(message, duration = 3000) {
            toast.textContent = message;
            toast.classList.add('show');
            setTimeout(() => toast.classList.remove('show'), duration);
        }

        // Initialize Firebase (if config is set)
        function initFirebase() {
            if (firebaseConfig.apiKey === "YOUR_API_KEY") {
                console.log('Firebase not configured - using browser notifications only');
                return;
            }

            try {
                firebase.initializeApp(firebaseConfig);
                messaging = firebase.messaging();
                
                // Handle foreground messages
                messaging.onMessage((payload) => {
                    console.log('Message received:', payload);
                    showToast(payload.notification.body);
                    
                    // Play alert sound
                    playAlertSound();
                    
                    // Show browser notification
                    if (Notification.permission === 'granted') {
                        new Notification(payload.notification.title, {
                            body: payload.notification.body,
                            icon: '/icons/icon-192.png',
                            vibrate: [200, 100, 200]
                        });
                    }
                });
            } catch (error) {
                console.log('Firebase init error:', error);
            }
        }

        // Request Notification Permission
        async function requestNotificationPermission() {
            if (!('Notification' in window)) {
                showToast('Browser tidak support notifikasi');
                return;
            }

            const permission = await Notification.requestPermission();
            
            if (permission === 'granted') {
                notificationEnabled = true;
                notifBtn.classList.add('notif-active');
                notifBanner.classList.remove('show');
                showToast('✅ Notifikasi diaktifkan!');
                
                // Get FCM Token if Firebase is configured
                if (messaging) {
                    try {
                        const token = await messaging.getToken();
                        console.log('FCM Token:', token);
                        // Send token to server
                        await saveFcmToken(token);
                    } catch (error) {
                        console.log('FCM Token error:', error);
                    }
                }
                
                // Test notification
                new Notification('Gas Monitor', {
                    body: 'Notifikasi aktif! Anda akan menerima peringatan saat ada kebocoran gas.',
                    icon: '/icons/icon-192.png'
                });
            } else {
                showToast('❌ Notifikasi ditolak');
            }
        }

        // Save FCM Token to server
        async function saveFcmToken(token) {
            try {
                await fetch(`${API_BASE}/api/save-fcm-token`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'X-CSRF-TOKEN': csrfToken
                    },
                    body: JSON.stringify({ token })
                });
            } catch (error) {
                console.log('Save token error:', error);
            }
        }

        // Play Alert Sound
        function playAlertSound() {
            const audio = new Audio('data:audio/wav;base64,UklGRnoGAABXQVZFZm10IBAAAAABAAEAQB8AAEAfAAABAAgAZGF0YQoGAACBhYqFbF1fdJivrJBhNjVgodDbq2EcBj+a2teleQcmgbqsm4B7Yn+dlaSfkHNfXXGSpamjlIMzAA==');
            audio.play().catch(() => {});
        }

        // Initialize Chart
        function initChart() {
            const ctx = document.getElementById('gasChart').getContext('2d');
            chart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Kadar Gas (%)',
                        data: [],
                        borderColor: '#00d9ff',
                        backgroundColor: 'rgba(0, 217, 255, 0.1)',
                        borderWidth: 2,
                        fill: true,
                        tension: 0.4,
                        pointRadius: 0,
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: { display: false }
                    },
                    scales: {
                        x: {
                            grid: { color: 'rgba(255,255,255,0.05)' },
                            ticks: { color: '#a0a0a0', maxTicksLimit: 6 }
                        },
                        y: {
                            min: 0,
                            max: 100,
                            grid: { color: 'rgba(255,255,255,0.05)' },
                            ticks: { color: '#a0a0a0' }
                        }
                    }
                }
            });
        }

        // Update Gauge
        function updateGauge(value) {
            gasValue.textContent = value;

            // Calculate stroke offset (0-100% maps to 251.2-0)
            const offset = 251.2 - (value / 100 * 251.2);
            gaugeFill.style.strokeDashoffset = offset;

            // Update color based on value
            gaugeFill.classList.remove('warning', 'danger');
            if (value > 70) {
                gaugeFill.classList.add('danger');
            } else if (value > 40) {
                gaugeFill.classList.add('warning');
            }
        }

        // Fetch Latest Reading
        let lastAlertState = false;
        async function fetchLatest() {
            try {
                const response = await fetch(`${API_BASE}/api/readings/latest`);
                const data = await response.json();

                if (data.success) {
                    updateGauge(data.data.gas);

                    // Update online status
                    isOnline = data.is_online;
                    statusDot.classList.toggle('online', isOnline);

                    // Update alert banner
                    const isAlert = data.data.is_alert;
                    alertBanner.classList.toggle('show', isAlert);

                    // Trigger notification on new alert
                    if (isAlert && !lastAlertState && notificationEnabled) {
                        playAlertSound();
                        if (Notification.permission === 'granted') {
                            new Notification('⚠️ BAHAYA! Kebocoran Gas!', {
                                body: `Kadar gas: ${data.data.gas}% - Segera periksa!`,
                                icon: '/icons/icon-192.png',
                                vibrate: [200, 100, 200, 100, 200],
                                tag: 'gas-alert',
                                requireInteraction: true
                            });
                        }
                    }
                    lastAlertState = isAlert;

                    // Update mode buttons
                    const isAuto = data.settings.mode === 'auto';
                    modeAuto.classList.toggle('active', isAuto);
                    modeManual.classList.toggle('active', !isAuto);

                    // Disable valve controls in auto mode
                    valveOn.disabled = isAuto;
                    valveOff.disabled = isAuto;

                    // Update valve buttons
                    const isValveOn = data.settings.valve_status === 'on';
                    valveOn.classList.toggle('active', isValveOn);
                    valveOff.classList.toggle('active', !isValveOn);

                    // Update connection status
                    connectionStatus.textContent = isOnline
                        ? `Terhubung • Terakhir update: ${new Date().toLocaleTimeString('id-ID')}`
                        : 'Perangkat tidak terhubung';
                }
            } catch (error) {
                console.error('Error fetching latest:', error);
                connectionStatus.textContent = 'Gagal mengambil data';
            }
        }

        // Fetch History for Chart
        async function fetchHistory(hours = 6) {
            try {
                const response = await fetch(`${API_BASE}/api/readings/history?hours=${hours}`);
                const data = await response.json();

                if (data.success && data.data.length > 0) {
                    noData.style.display = 'none';
                    document.getElementById('gasChart').style.display = 'block';
                    chart.data.labels = data.data.map(d => d.timestamp);
                    chart.data.datasets[0].data = data.data.map(d => d.gas);
                    chart.update();
                } else {
                    noData.style.display = 'flex';
                    document.getElementById('gasChart').style.display = 'none';
                }
            } catch (error) {
                console.error('Error fetching history:', error);
            }
        }

        // Set Mode
        async function setMode(mode) {
            showLoading(true);
            try {
                const response = await fetch(`${API_BASE}/api/control/mode`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'X-CSRF-TOKEN': csrfToken
                    },
                    body: JSON.stringify({ mode })
                });
                const data = await response.json();
                if (data.success) {
                    modeAuto.classList.toggle('active', mode === 'auto');
                    modeManual.classList.toggle('active', mode === 'manual');
                    valveOn.disabled = mode === 'auto';
                    valveOff.disabled = mode === 'auto';
                    showToast(`Mode: ${mode.toUpperCase()}`);
                }
            } catch (error) {
                console.error('Error setting mode:', error);
                showToast('Gagal mengubah mode');
            }
            showLoading(false);
        }

        // Control Valve
        async function controlValve(status) {
            showLoading(true);
            try {
                const response = await fetch(`${API_BASE}/api/control/valve`, {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json',
                        'X-CSRF-TOKEN': csrfToken
                    },
                    body: JSON.stringify({ status })
                });
                const data = await response.json();
                if (data.success) {
                    valveOn.classList.toggle('active', status === 'on');
                    valveOff.classList.toggle('active', status === 'off');
                    showToast(`Komponen: ${status.toUpperCase()}`);
                }
            } catch (error) {
                console.error('Error controlling valve:', error);
                showToast('Gagal mengontrol komponen');
            }
            showLoading(false);
        }

        // Show/Hide Loading
        function showLoading(show) {
            loadingOverlay.classList.toggle('show', show);
        }

        // PWA Install
        window.addEventListener('beforeinstallprompt', (e) => {
            e.preventDefault();
            deferredPrompt = e;
            installBtn.classList.add('show');
        });

        installBtn.addEventListener('click', async () => {
            if (deferredPrompt) {
                deferredPrompt.prompt();
                const result = await deferredPrompt.userChoice;
                if (result.outcome === 'accepted') {
                    installBtn.classList.remove('show');
                    showToast('Aplikasi berhasil diinstall!');
                }
                deferredPrompt = null;
            }
        });

        // Event Listeners
        modeAuto.addEventListener('click', () => setMode('auto'));
        modeManual.addEventListener('click', () => setMode('manual'));
        valveOn.addEventListener('click', () => !valveOn.disabled && controlValve('on'));
        valveOff.addEventListener('click', () => !valveOff.disabled && controlValve('off'));

        document.getElementById('refreshBtn').addEventListener('click', () => {
            fetchLatest();
            fetchHistory(currentPeriod);
            showToast('Data diperbarui');
        });

        document.querySelectorAll('.period-btn').forEach(btn => {
            btn.addEventListener('click', () => {
                document.querySelectorAll('.period-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                currentPeriod = parseInt(btn.dataset.hours);
                fetchHistory(currentPeriod);
            });
        });

        // Notification buttons
        notifBtn.addEventListener('click', requestNotificationPermission);
        notifBanner.addEventListener('click', requestNotificationPermission);

        // Info button
        document.getElementById('infoBtn').addEventListener('click', () => {
            alert('Gas Monitor PWA v1.0\n\nSistem Pendeteksi Kebocoran Gas LPG\nBerbasis Internet of Things (IoT)\n\n© 2026 SKRIPSI');
        });

        // Initialize
        document.addEventListener('DOMContentLoaded', () => {
            initChart();
            initFirebase();
            fetchLatest();
            fetchHistory(currentPeriod);

            // Check notification permission
            if ('Notification' in window) {
                if (Notification.permission === 'granted') {
                    notificationEnabled = true;
                    notifBtn.classList.add('notif-active');
                } else if (Notification.permission === 'default') {
                    notifBanner.classList.add('show');
                }
            }

            // Start polling
            setInterval(fetchLatest, POLLING_INTERVAL);
            setInterval(() => fetchHistory(currentPeriod), 30000);
        });

        // Register Service Worker
        if ('serviceWorker' in navigator) {
            navigator.serviceWorker.register('/sw.js')
                .then(reg => console.log('SW registered'))
                .catch(err => console.log('SW registration failed:', err));
        }
    </script>
</body>

</html>