<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class GasReading extends Model
{
    protected $fillable = [
        'gas_percentage',
        'voltage',
        'is_alert',
    ];

    protected $casts = [
        'is_alert' => 'boolean',
        'voltage' => 'float',
    ];

    /**
     * Get latest reading
     */
    public static function latest()
    {
        return self::orderBy('created_at', 'desc')->first();
    }

    /**
     * Get readings for chart (last N hours)
     */
    public static function history($hours = 6)
    {
        return self::where('created_at', '>=', now()->subHours($hours))
            ->orderBy('created_at', 'asc')
            ->get();
    }
}
