<?php

use App\Http\Controllers\Api\DeviceController;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| API Routes for ESP32 Device
|--------------------------------------------------------------------------
*/

// ESP32 sends sensor data
Route::post('/device/data', [DeviceController::class, 'storeData']);

// ESP32 gets pending commands
Route::get('/device/commands', [DeviceController::class, 'getCommands']);

// ESP32 acknowledges command execution
Route::post('/device/command-ack', [DeviceController::class, 'acknowledgeCommand']);

// Get device status
Route::get('/device/status', [DeviceController::class, 'getStatus']);
