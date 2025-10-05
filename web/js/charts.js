/**
 * ================================================================
 * CHARTS AND VISUALIZATIONS
 * Advanced charting system for Triple Mining Platform
 * Using Chart.js for real-time data visualization
 * ================================================================
 */

class ChartsManager {
    constructor(app) {
        this.app = app;
        this.charts = {};
        this.chartConfigs = {};
        this.updateIntervals = {};
        this.dataBuffers = {};
        
        // Chart color schemes
        this.colors = {
            sha256: '#ff6b35',
            biological: '#4ecdc4',
            mea: '#45b7d1',
            success: '#4caf50',
            warning: '#ff9800',
            error: '#f44336',
            info: '#2196f3',
            accent: '#00ff88'
        };
        
        this.init();
    }

    /**
     * Initialize charts manager
     */
    init() {
        // Wait for DOM to be ready
        if (document.readyState === 'loading') {
            document.addEventListener('DOMContentLoaded', () => {
                this.setupCharts();
            });
        } else {
            this.setupCharts();
        }
    }

    /**
     * Setup all charts
     */
    setupCharts() {
        console.log('📊 Initializing charts...');
        
        // Initialize data buffers
        this.initializeDataBuffers();
        
        // Create dashboard charts
        this.createHashrateChart();
        this.createPerformanceChart();
        this.createElectrodeActivityChart();
        
        // Create mining charts
        this.createMiningProgressChart();
        this.createNonceSuccessChart();
        this.createSystemComparisonChart();
        
        // Create results charts
        this.createLearningCurveChart();
        this.createAccuracyComparisonChart();
        this.createRealTimeMetricsChart();
        
        console.log('✅ Charts initialized successfully');
    }

    /**
     * Initialize data buffers for real-time updates
     */
    initializeDataBuffers() {
        const bufferSize = 100; // Keep last 100 data points
        
        this.dataBuffers = {
            hashrate: {
                sha256: new Array(bufferSize).fill(0),
                biological: new Array(bufferSize).fill(0),
                mea: new Array(bufferSize).fill(0),
                timestamps: new Array(bufferSize).fill(new Date())
            },
            performance: {
                cpu: new Array(bufferSize).fill(0),
                memory: new Array(bufferSize).fill(0),
                gpu: new Array(bufferSize).fill(0),
                timestamps: new Array(bufferSize).fill(new Date())
            },
            electrodes: {
                active: new Array(bufferSize).fill(0),
                stimulating: new Array(bufferSize).fill(0),
                recording: new Array(bufferSize).fill(0),
                timestamps: new Array(bufferSize).fill(new Date())
            },
            mining: {
                attempts: new Array(bufferSize).fill(0),
                successes: new Array(bufferSize).fill(0),
                timestamps: new Array(bufferSize).fill(new Date())
            },
            learning: {
                biological_loss: new Array(bufferSize).fill(1),
                mea_loss: new Array(bufferSize).fill(1),
                biological_accuracy: new Array(bufferSize).fill(0),
                mea_accuracy: new Array(bufferSize).fill(0),
                epochs: new Array(bufferSize).fill(0)
            }
        };
    }

    /**
     * Create hashrate comparison chart
     */
    createHashrateChart() {
        const ctx = document.getElementById('hashrate-chart');
        if (!ctx) return;

        const config = {
            type: 'line',
            data: {
                labels: this.dataBuffers.hashrate.timestamps.map(t => 
                    t.toLocaleTimeString()
                ),
                datasets: [
                    {
                        label: 'SHA-256 Mining',
                        data: this.dataBuffers.hashrate.sha256,
                        borderColor: this.colors.sha256,
                        backgroundColor: this.colors.sha256 + '20',
                        fill: true,
                        tension: 0.4
                    },
                    {
                        label: 'Biological Network',
                        data: this.dataBuffers.hashrate.biological,
                        borderColor: this.colors.biological,
                        backgroundColor: this.colors.biological + '20',
                        fill: true,
                        tension: 0.4
                    },
                    {
                        label: 'MEA Interface',
                        data: this.dataBuffers.hashrate.mea,
                        borderColor: this.colors.mea,
                        backgroundColor: this.colors.mea + '20',
                        fill: true,
                        tension: 0.4
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'System Hashrate Comparison (MH/s)',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: {
                            color: '#cccccc',
                            callback: function(value) {
                                return value.toFixed(2) + ' MH/s';
                            }
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    x: {
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                },
                animation: {
                    duration: 0 // Disable animation for real-time updates
                }
            }
        };

        this.charts.hashrate = new Chart(ctx, config);
    }

    /**
     * Create system performance chart
     */
    createPerformanceChart() {
        const ctx = document.getElementById('performance-chart');
        if (!ctx) return;

        const config = {
            type: 'line',
            data: {
                labels: this.dataBuffers.performance.timestamps.map(t => 
                    t.toLocaleTimeString()
                ),
                datasets: [
                    {
                        label: 'CPU Usage',
                        data: this.dataBuffers.performance.cpu,
                        borderColor: this.colors.error,
                        backgroundColor: this.colors.error + '20',
                        fill: false
                    },
                    {
                        label: 'Memory Usage',
                        data: this.dataBuffers.performance.memory,
                        borderColor: this.colors.warning,
                        backgroundColor: this.colors.warning + '20',
                        fill: false
                    },
                    {
                        label: 'GPU Usage',
                        data: this.dataBuffers.performance.gpu,
                        borderColor: this.colors.accent,
                        backgroundColor: this.colors.accent + '20',
                        fill: false
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'System Performance Metrics (%)',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        ticks: {
                            color: '#cccccc',
                            callback: function(value) {
                                return value + '%';
                            }
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    x: {
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                }
            }
        };

        this.charts.performance = new Chart(ctx, config);
    }

    /**
     * Create electrode activity chart
     */
    createElectrodeActivityChart() {
        const ctx = document.getElementById('electrode-activity-chart');
        if (!ctx) return;

        const config = {
            type: 'doughnut',
            data: {
                labels: ['Active', 'Inactive', 'Stimulating', 'Recording'],
                datasets: [{
                    data: [20, 40, 5, 15], // Initial values
                    backgroundColor: [
                        this.colors.success,
                        '#666666',
                        this.colors.warning,
                        this.colors.info
                    ],
                    borderWidth: 2,
                    borderColor: '#333333'
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'MEA Electrode Status Distribution',
                        color: '#ffffff'
                    },
                    legend: {
                        position: 'bottom',
                        labels: {
                            color: '#ffffff',
                            padding: 20
                        }
                    }
                }
            }
        };

        this.charts.electrodeActivity = new Chart(ctx, config);
    }

    /**
     * Create mining progress chart
     */
    createMiningProgressChart() {
        const ctx = document.getElementById('mining-progress-chart');
        if (!ctx) return;

        const config = {
            type: 'bar',
            data: {
                labels: ['SHA-256', 'Biological', 'MEA', 'Combined'],
                datasets: [
                    {
                        label: 'Mining Attempts',
                        data: [0, 0, 0, 0],
                        backgroundColor: [
                            this.colors.sha256 + '80',
                            this.colors.biological + '80',
                            this.colors.mea + '80',
                            this.colors.accent + '80'
                        ],
                        borderColor: [
                            this.colors.sha256,
                            this.colors.biological,
                            this.colors.mea,
                            this.colors.accent
                        ],
                        borderWidth: 2
                    },
                    {
                        label: 'Successful Nonces',
                        data: [0, 0, 0, 0],
                        backgroundColor: [
                            this.colors.success + '80',
                            this.colors.success + '80',
                            this.colors.success + '80',
                            this.colors.success + '80'
                        ],
                        borderColor: this.colors.success,
                        borderWidth: 2
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'Mining Performance by System',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    x: {
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                }
            }
        };

        this.charts.miningProgress = new Chart(ctx, config);
    }

    /**
     * Create nonce success rate chart
     */
    createNonceSuccessChart() {
        const ctx = document.getElementById('nonce-success-chart');
        if (!ctx) return;

        const config = {
            type: 'line',
            data: {
                labels: this.dataBuffers.mining.timestamps.map(t => 
                    t.toLocaleTimeString()
                ),
                datasets: [
                    {
                        label: 'Success Rate (%)',
                        data: this.dataBuffers.mining.successes.map((s, i) => 
                            this.dataBuffers.mining.attempts[i] > 0 ? 
                            (s / this.dataBuffers.mining.attempts[i]) * 100 : 0
                        ),
                        borderColor: this.colors.success,
                        backgroundColor: this.colors.success + '20',
                        fill: true,
                        tension: 0.4
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'Nonce Success Rate Over Time',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100,
                        ticks: {
                            color: '#cccccc',
                            callback: function(value) {
                                return value.toFixed(1) + '%';
                            }
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    x: {
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                }
            }
        };

        this.charts.nonceSuccess = new Chart(ctx, config);
    }

    /**
     * Create system comparison chart
     */
    createSystemComparisonChart() {
        const ctx = document.getElementById('system-comparison-chart');
        if (!ctx) return;

        const config = {
            type: 'radar',
            data: {
                labels: [
                    'Speed',
                    'Accuracy',
                    'Efficiency',
                    'Adaptability',
                    'Learning',
                    'Stability'
                ],
                datasets: [
                    {
                        label: 'SHA-256',
                        data: [9, 8, 7, 3, 2, 9],
                        borderColor: this.colors.sha256,
                        backgroundColor: this.colors.sha256 + '30',
                        pointBackgroundColor: this.colors.sha256
                    },
                    {
                        label: 'Biological Network',
                        data: [6, 7, 8, 9, 9, 6],
                        borderColor: this.colors.biological,
                        backgroundColor: this.colors.biological + '30',
                        pointBackgroundColor: this.colors.biological
                    },
                    {
                        label: 'MEA Interface',
                        data: [5, 8, 6, 8, 8, 7],
                        borderColor: this.colors.mea,
                        backgroundColor: this.colors.mea + '30',
                        pointBackgroundColor: this.colors.mea
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'System Capabilities Comparison',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    r: {
                        beginAtZero: true,
                        max: 10,
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        },
                        pointLabels: {
                            color: '#ffffff'
                        }
                    }
                }
            }
        };

        this.charts.systemComparison = new Chart(ctx, config);
    }

    /**
     * Create learning curve chart
     */
    createLearningCurveChart() {
        const ctx = document.getElementById('learning-curve-chart');
        if (!ctx) return;

        const config = {
            type: 'line',
            data: {
                labels: this.dataBuffers.learning.epochs,
                datasets: [
                    {
                        label: 'Biological Network Loss',
                        data: this.dataBuffers.learning.biological_loss,
                        borderColor: this.colors.biological,
                        backgroundColor: this.colors.biological + '20',
                        yAxisID: 'y'
                    },
                    {
                        label: 'MEA Interface Loss',
                        data: this.dataBuffers.learning.mea_loss,
                        borderColor: this.colors.mea,
                        backgroundColor: this.colors.mea + '20',
                        yAxisID: 'y'
                    },
                    {
                        label: 'Biological Accuracy',
                        data: this.dataBuffers.learning.biological_accuracy,
                        borderColor: this.colors.success,
                        backgroundColor: this.colors.success + '20',
                        yAxisID: 'y1'
                    },
                    {
                        label: 'MEA Accuracy',
                        data: this.dataBuffers.learning.mea_accuracy,
                        borderColor: this.colors.info,
                        backgroundColor: this.colors.info + '20',
                        yAxisID: 'y1'
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'Learning Progress - Loss and Accuracy',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Loss',
                            color: '#ffffff'
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Accuracy',
                            color: '#ffffff'
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            drawOnChartArea: false
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Epoch',
                            color: '#ffffff'
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                }
            }
        };

        this.charts.learningCurve = new Chart(ctx, config);
    }

    /**
     * Create accuracy comparison chart
     */
    createAccuracyComparisonChart() {
        const ctx = document.getElementById('accuracy-comparison-chart');
        if (!ctx) return;

        const config = {
            type: 'polarArea',
            data: {
                labels: ['SHA-256 Precision', 'Biological Learning', 'MEA Adaptation', 'Combined Accuracy'],
                datasets: [{
                    data: [95, 87, 82, 96],
                    backgroundColor: [
                        this.colors.sha256 + '80',
                        this.colors.biological + '80',
                        this.colors.mea + '80',
                        this.colors.accent + '80'
                    ],
                    borderColor: [
                        this.colors.sha256,
                        this.colors.biological,
                        this.colors.mea,
                        this.colors.accent
                    ],
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'System Accuracy Comparison (%)',
                        color: '#ffffff'
                    },
                    legend: {
                        position: 'bottom',
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    r: {
                        beginAtZero: true,
                        max: 100,
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    }
                }
            }
        };

        this.charts.accuracyComparison = new Chart(ctx, config);
    }

    /**
     * Create real-time metrics chart
     */
    createRealTimeMetricsChart() {
        const ctx = document.getElementById('realtime-metrics-chart');
        if (!ctx) return;

        const config = {
            type: 'line',
            data: {
                datasets: [
                    {
                        label: 'Network Latency (ms)',
                        data: [],
                        borderColor: this.colors.info,
                        backgroundColor: this.colors.info + '20',
                        yAxisID: 'y'
                    },
                    {
                        label: 'Throughput (tx/s)',
                        data: [],
                        borderColor: this.colors.success,
                        backgroundColor: this.colors.success + '20',
                        yAxisID: 'y1'
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    title: {
                        display: true,
                        text: 'Real-Time Network Metrics',
                        color: '#ffffff'
                    },
                    legend: {
                        labels: {
                            color: '#ffffff'
                        }
                    }
                },
                scales: {
                    x: {
                        type: 'realtime',
                        realtime: {
                            duration: 20000,
                            refresh: 1000,
                            delay: 500
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        title: {
                            display: true,
                            text: 'Latency (ms)',
                            color: '#ffffff'
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            color: 'rgba(255, 255, 255, 0.1)'
                        }
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        title: {
                            display: true,
                            text: 'Throughput (tx/s)',
                            color: '#ffffff'
                        },
                        ticks: {
                            color: '#cccccc'
                        },
                        grid: {
                            drawOnChartArea: false
                        }
                    }
                },
                animation: {
                    duration: 0
                }
            }
        };

        // Note: This would require Chart.js streaming plugin
        // this.charts.realtimeMetrics = new Chart(ctx, config);
    }

    /**
     * Update chart with new data
     */
    updateChart(chartName, newData) {
        const chart = this.charts[chartName];
        if (!chart) return;

        switch (chartName) {
            case 'hashrate':
                this.updateHashrateChart(newData);
                break;
            case 'performance':
                this.updatePerformanceChart(newData);
                break;
            case 'electrodeActivity':
                this.updateElectrodeActivityChart(newData);
                break;
            case 'miningProgress':
                this.updateMiningProgressChart(newData);
                break;
            case 'nonceSuccess':
                this.updateNonceSuccessChart(newData);
                break;
            case 'learningCurve':
                this.updateLearningCurveChart(newData);
                break;
        }
    }

    /**
     * Update hashrate chart with new data
     */
    updateHashrateChart(data) {
        const chart = this.charts.hashrate;
        const buffer = this.dataBuffers.hashrate;

        // Shift arrays and add new data
        buffer.sha256.shift();
        buffer.sha256.push(data.sha256_hashrate || 0);
        
        buffer.biological.shift();
        buffer.biological.push(data.biological_hashrate || 0);
        
        buffer.mea.shift();
        buffer.mea.push(data.mea_hashrate || 0);
        
        buffer.timestamps.shift();
        buffer.timestamps.push(new Date());

        // Update chart
        chart.data.labels = buffer.timestamps.map(t => t.toLocaleTimeString());
        chart.data.datasets[0].data = buffer.sha256;
        chart.data.datasets[1].data = buffer.biological;
        chart.data.datasets[2].data = buffer.mea;
        
        chart.update('none');
    }

    /**
     * Update performance chart with new data
     */
    updatePerformanceChart(data) {
        const chart = this.charts.performance;
        const buffer = this.dataBuffers.performance;

        // Shift arrays and add new data
        buffer.cpu.shift();
        buffer.cpu.push(data.cpu_usage || 0);
        
        buffer.memory.shift();
        buffer.memory.push(data.memory_usage || 0);
        
        buffer.gpu.shift();
        buffer.gpu.push(data.gpu_usage || 0);
        
        buffer.timestamps.shift();
        buffer.timestamps.push(new Date());

        // Update chart
        chart.data.labels = buffer.timestamps.map(t => t.toLocaleTimeString());
        chart.data.datasets[0].data = buffer.cpu;
        chart.data.datasets[1].data = buffer.memory;
        chart.data.datasets[2].data = buffer.gpu;
        
        chart.update('none');
    }

    /**
     * Update electrode activity chart
     */
    updateElectrodeActivityChart(data) {
        const chart = this.charts.electrodeActivity;
        
        chart.data.datasets[0].data = [
            data.active || 0,
            (60 - (data.active || 0)),
            data.stimulating || 0,
            data.recording || 0
        ];
        
        chart.update('none');
    }

    /**
     * Update mining progress chart
     */
    updateMiningProgressChart(data) {
        const chart = this.charts.miningProgress;
        
        chart.data.datasets[0].data = [
            data.sha256_attempts || 0,
            data.biological_attempts || 0,
            data.mea_attempts || 0,
            data.total_attempts || 0
        ];
        
        chart.data.datasets[1].data = [
            data.sha256_successes || 0,
            data.biological_successes || 0,
            data.mea_successes || 0,
            data.total_successes || 0
        ];
        
        chart.update('none');
    }

    /**
     * Update nonce success chart
     */
    updateNonceSuccessChart(data) {
        const chart = this.charts.nonceSuccess;
        const buffer = this.dataBuffers.mining;

        // Calculate success rate
        const successRate = data.attempts > 0 ? (data.successes / data.attempts) * 100 : 0;
        
        // Shift arrays and add new data
        buffer.attempts.shift();
        buffer.attempts.push(data.attempts || 0);
        
        buffer.successes.shift();
        buffer.successes.push(data.successes || 0);
        
        buffer.timestamps.shift();
        buffer.timestamps.push(new Date());

        // Update chart
        chart.data.labels = buffer.timestamps.map(t => t.toLocaleTimeString());
        chart.data.datasets[0].data = buffer.successes.map((s, i) => 
            buffer.attempts[i] > 0 ? (s / buffer.attempts[i]) * 100 : 0
        );
        
        chart.update('none');
    }

    /**
     * Update learning curve chart
     */
    updateLearningCurveChart(data) {
        const chart = this.charts.learningCurve;
        const buffer = this.dataBuffers.learning;

        if (data.epoch !== undefined) {
            // Add new training data
            buffer.epochs.push(data.epoch);
            buffer.biological_loss.push(data.biological_loss || 1);
            buffer.mea_loss.push(data.mea_loss || 1);
            buffer.biological_accuracy.push(data.biological_accuracy || 0);
            buffer.mea_accuracy.push(data.mea_accuracy || 0);

            // Keep only last 100 epochs
            if (buffer.epochs.length > 100) {
                buffer.epochs.shift();
                buffer.biological_loss.shift();
                buffer.mea_loss.shift();
                buffer.biological_accuracy.shift();
                buffer.mea_accuracy.shift();
            }

            // Update chart
            chart.data.labels = buffer.epochs;
            chart.data.datasets[0].data = buffer.biological_loss;
            chart.data.datasets[1].data = buffer.mea_loss;
            chart.data.datasets[2].data = buffer.biological_accuracy;
            chart.data.datasets[3].data = buffer.mea_accuracy;
            
            chart.update('none');
        }
    }

    /**
     * Resize all charts
     */
    resizeCharts() {
        Object.values(this.charts).forEach(chart => {
            if (chart && chart.resize) {
                chart.resize();
            }
        });
    }

    /**
     * Destroy all charts
     */
    destroy() {
        Object.values(this.charts).forEach(chart => {
            if (chart && chart.destroy) {
                chart.destroy();
            }
        });
        
        // Clear update intervals
        Object.values(this.updateIntervals).forEach(interval => {
            clearInterval(interval);
        });
        
        this.charts = {};
        this.updateIntervals = {};
    }

    /**
     * Get chart instance
     */
    getChart(name) {
        return this.charts[name];
    }

    /**
     * Get all chart instances
     */
    getAllCharts() {
        return this.charts;
    }
}

// Export for module usage
if (typeof module !== 'undefined' && module.exports) {
    module.exports = ChartsManager;
}