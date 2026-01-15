<?php

namespace App\Http\Controllers;

use App\Models\DeviceCommand;
use App\Models\DeviceSetting;
use App\Models\GasReading;
use Illuminate\Http\Request;

class DashboardController extends Controller
{
    /**
     * Show main dashboard
     */
    public function index()
    {
        $latestReading = GasReading::orderBy('created_at', 'desc')->first();
        $settings = DeviceSetting::allAsArray();

        return view('dashboard', [
            'latestReading' => $latestReading,
            'settings' => $settings,
        ]);
    }

    /**
     * Get latest reading for AJAX polling
     * GET /api/readings/latest
     */
    public function latestReading()
    {
        $reading = GasReading::orderBy('created_at', 'desc')->first();
        $lastSeen = DeviceSetting::get('last_seen');
        $isOnline = false;

        if ($lastSeen) {
            $lastSeenTime = \Carbon\Carbon::parse($lastSeen);
            $isOnline = $lastSeenTime->diffInSeconds(now()) < 10;
        }

        return response()->json([
            'success' => true,
            'data' => [
                'gas' => $reading ? $reading->gas_percentage : 0,
                'voltage' => $reading ? $reading->voltage : 0,
                'is_alert' => $reading ? $reading->is_alert : false,
                'timestamp' => $reading ? $reading->created_at->toISOString() : null,
            ],
            'settings' => [
                'mode' => DeviceSetting::get('mode', 'auto'),
                'valve_status' => DeviceSetting::get('valve_status', 'on'),
            ],
            'is_online' => $isOnline,
        ]);
    }

    /**
     * Get history data for chart
     * GET /api/readings/history
     */
    public function history(Request $request)
    {
        $hours = $request->get('hours', 6);
        $readings = GasReading::where('created_at', '>=', now()->subHours($hours))
            ->orderBy('created_at', 'asc')
            ->get();

        return response()->json([
            'success' => true,
            'data' => $readings->map(function ($r) {
                return [
                    'gas' => $r->gas_percentage,
                    'timestamp' => $r->created_at->format('H:i'),
                    'is_alert' => $r->is_alert,
                ];
            }),
        ]);
    }

    /**
     * Set mode (auto/manual)
     * POST /api/control/mode
     */
    public function setMode(Request $request)
    {
        $validated = $request->validate([
            'mode' => 'required|in:auto,manual',
        ]);

        DeviceSetting::set('mode', $validated['mode']);

        // Create command for ESP32
        DeviceCommand::create([
            'command' => 'mode_' . $validated['mode'],
        ]);

        return response()->json([
            'success' => true,
            'mode' => $validated['mode'],
        ]);
    }

    /**
     * Control valve (on/off)
     * POST /api/control/valve
     */
    public function controlValve(Request $request)
    {
        $validated = $request->validate([
            'status' => 'required|in:on,off',
        ]);

        DeviceSetting::set('valve_status', $validated['status']);

        // Create command for ESP32
        DeviceCommand::create([
            'command' => 'valve_' . $validated['status'],
        ]);

        return response()->json([
            'success' => true,
            'valve_status' => $validated['status'],
        ]);
    }
}
