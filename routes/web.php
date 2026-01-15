<?php

use App\Http\Controllers\DashboardController;
use Illuminate\Support\Facades\Route;

/*
|--------------------------------------------------------------------------
| Web Routes for PWA Dashboard
|--------------------------------------------------------------------------
*/

// Main Dashboard
Route::get('/', [DashboardController::class, 'index'])->name('dashboard');

// API for PWA (AJAX calls)
Route::prefix('api')->group(function () {
    Route::get('/readings/latest', [DashboardController::class, 'latestReading']);
    Route::get('/readings/history', [DashboardController::class, 'history']);
    Route::post('/control/mode', [DashboardController::class, 'setMode']);
    Route::post('/control/valve', [DashboardController::class, 'controlValve']);
});
