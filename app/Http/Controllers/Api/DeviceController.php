<?php

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\DeviceCommand;
use App\Models\DeviceSetting;
use App\Models\GasReading;
use Illuminate\Http\Request;

class DeviceController extends Controller
{
    /**
     * Receive data from ESP32
     * POST /api/device/data
     */
    public function storeData(Request $request)
    {
        $validated = $request->validate([
            'gas' => 'required|integer|min:0|max:100',
            'voltage' => 'required|numeric',
            'alert' => 'boolean',
        ]);

        // Store gas reading
        $reading = GasReading::create([
            'gas_percentage' => $validated['gas'],
            'voltage' => $validated['voltage'],
            'is_alert' => $validated['alert'] ?? false,
        ]);

        // Update last seen
        DeviceSetting::set('last_seen', now()->toISOString());
        DeviceSetting::set('last_gas', $validated['gas']);
        DeviceSetting::set('is_online', 'true');

        // If alert, we could trigger push notification here
        if ($validated['alert'] ?? false) {
            DeviceSetting::set('last_alert', now()->toISOString());
        }

        return response()->json([
            'success' => true,
            'reading_id' => $reading->id,
            'mode' => DeviceSetting::get('mode', 'auto'),
        ]);
    }

    /**
     * Get pending commands for ESP32
     * GET /api/device/commands
     */
    public function getCommands()
    {
        $commands = DeviceCommand::pending();

        return response()->json([
            'success' => true,
            'commands' => $commands->map(function ($cmd) {
                return [
                    'id' => $cmd->id,
                    'command' => $cmd->command,
                ];
            }),
        ]);
    }

    /**
     * Acknowledge command execution
     * POST /api/device/command-ack
     */
    public function acknowledgeCommand(Request $request)
    {
        $validated = $request->validate([
            'command_id' => 'required|integer|exists:device_commands,id',
        ]);

        $command = DeviceCommand::find($validated['command_id']);
        if ($command) {
            $command->markExecuted();
        }

        return response()->json([
            'success' => true,
        ]);
    }

    /**
     * Get device status
     * GET /api/device/status
     */
    public function getStatus()
    {
        $lastSeen = DeviceSetting::get('last_seen');
        $isOnline = false;

        if ($lastSeen) {
            $lastSeenTime = \Carbon\Carbon::parse($lastSeen);
            $isOnline = $lastSeenTime->diffInSeconds(now()) < 10; // Online if last seen within 10 seconds
        }

        return response()->json([
            'success' => true,
            'mode' => DeviceSetting::get('mode', 'auto'),
            'valve_status' => DeviceSetting::get('valve_status', 'on'),
            'is_online' => $isOnline,
            'last_seen' => $lastSeen,
            'last_gas' => (int) DeviceSetting::get('last_gas', 0),
        ]);
    }
}
