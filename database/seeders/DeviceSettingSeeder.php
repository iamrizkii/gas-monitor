<?php

namespace Database\Seeders;

use App\Models\DeviceSetting;
use Illuminate\Database\Seeder;

class DeviceSettingSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        $settings = [
            ['key' => 'mode', 'value' => 'auto'],
            ['key' => 'valve_status', 'value' => 'on'],
            ['key' => 'is_online', 'value' => 'false'],
            ['key' => 'last_seen', 'value' => null],
            ['key' => 'last_gas', 'value' => '0'],
            ['key' => 'last_alert', 'value' => null],
        ];

        foreach ($settings as $setting) {
            DeviceSetting::updateOrCreate(
                ['key' => $setting['key']],
                ['value' => $setting['value']]
            );
        }
    }
}
