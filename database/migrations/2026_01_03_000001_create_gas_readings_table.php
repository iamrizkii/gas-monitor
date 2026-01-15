<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('gas_readings', function (Blueprint $table) {
            $table->id();
            $table->integer('gas_percentage')->default(0);
            $table->float('voltage', 8, 4)->default(0);
            $table->boolean('is_alert')->default(false);
            $table->timestamps();
            
            $table->index('created_at');
            $table->index('is_alert');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('gas_readings');
    }
};
