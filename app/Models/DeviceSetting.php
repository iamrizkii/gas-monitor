<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Model;

class DeviceSetting extends Model
{
    protected $fillable = [
        'key',
        'value',
    ];

    /**
     * Get setting by key
     */
    public static function get($key, $default = null)
    {
        $setting = self::where('key', $key)->first();
        return $setting ? $setting->value : $default;
    }

    /**
     * Set setting by key
     */
    public static function set($key, $value)
    {
        return self::updateOrCreate(
            ['key' => $key],
            ['value' => $value]
        );
    }

    /**
     * Get all settings as array
     */
    public static function allAsArray()
    {
        return self::pluck('value', 'key')->toArray();
    }
}
