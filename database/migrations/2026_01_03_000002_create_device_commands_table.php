<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration {
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('device_commands', function (Blueprint $table) {
            $table->id();
            $table->string('command'); // valve_on, valve_off, mode_auto, mode_manual
            $table->boolean('executed')->default(false);
            $table->timestamp('executed_at')->nullable();
            $table->timestamps();

            $table->index('executed');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('device_commands');
    }
};
