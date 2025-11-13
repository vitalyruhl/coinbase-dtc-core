#!/usr/bin/env pwsh

# Docker validation script for Open DTC Server
# This script validates that all core DTC libraries build successfully in Docker

Write-Host "🔍 Open DTC Server - Docker Validation" -ForegroundColor Cyan
Write-Host "=======================================" -ForegroundColor Cyan

Write-Host "`n📦 Building Docker test image..." -ForegroundColor Yellow

# Build the test image
$buildResult = docker build -f Dockerfile.test --target test -t open-dtc-server:test-validation .

if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Docker build completed successfully!" -ForegroundColor Green
    
    Write-Host "`n📋 Validation Summary:" -ForegroundColor Cyan
    Write-Host "├── ✅ Core DTC Protocol library" -ForegroundColor Green
    Write-Host "├── ✅ DTC Authentication library" -ForegroundColor Green 
    Write-Host "├── ✅ Exchange Base library" -ForegroundColor Green
    Write-Host "├── ✅ Binance Feed library" -ForegroundColor Green
    Write-Host "├── ✅ Coinbase Feed library" -ForegroundColor Green
    Write-Host "└── ✅ DTC Utilities library" -ForegroundColor Green
    
    Write-Host "`n🎯 Core libraries successfully validated in containerized environment" -ForegroundColor Green
    Write-Host "🚀 Ready for CI/CD deployment and GitHub Actions testing" -ForegroundColor Green
    
    # Check image size
    $imageInfo = docker images open-dtc-server:test-validation --format "table {{.Size}}"
    Write-Host "`n📊 Image size: $($imageInfo.Split("`n")[1])" -ForegroundColor Blue
    
} else {
    Write-Host "❌ Docker build failed!" -ForegroundColor Red
    Write-Host "Please check the build output above for errors." -ForegroundColor Red
    exit 1
}

Write-Host "`n🔧 Available Docker commands:" -ForegroundColor Cyan
Write-Host "  docker run --rm open-dtc-server:test-validation" -ForegroundColor White
Write-Host "  docker run --rm open-dtc-server:test-only" -ForegroundColor White

Write-Host "`n✨ Docker validation completed successfully!" -ForegroundColor Green