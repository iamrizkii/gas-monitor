<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class DeviceCommand extends Model
{
    protected $fillable = [
        'command',
        'executed',
        'executed_at',
    ];

    protected $casts = [
        'executed' => 'boolean',
        'executed_at' => 'datetime',
    ];

    /**
     * Get pending commands for ESP32
     */
    public static function pending()
    {
        return self::where('executed', false)
            ->orderBy('created_at', 'asc')
            ->get();
    }

    /**
     * Mark command as executed
     */
    public function markExecuted()
    {
        $this->update([
            'executed' => true,
            'executed_at' => now(),
        ]);
    }
}
