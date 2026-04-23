import matplotlib.pyplot as plt
import numpy as np

# Data from benchmark
labels = ['Scalar (sinf/cosf)', 'NEON (sincos_ps)']
throughput = [554.16, 1689.57]  # M/s
execution_time = [0.000121, 0.000040] # seconds

# Create the plot
fig, ax1 = plt.subplots(figsize=(10, 6))

# Throughput Bar Plot
color = 'tab:blue'
ax1.set_xlabel('Implementation')
ax1.set_ylabel('Throughput (Million elements/s)', color=color)
bars = ax1.bar(labels, throughput, color=['#A0C4FF', '#FFADAD'], alpha=0.7)
ax1.tick_params(axis='y', labelcolor=color)
ax1.set_title('sincos_acc: ARM NEON vs Scalar Performance (66,820 elements)', fontsize=14)

# Adding throughput labels on bars
for bar in bars:
    height = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2., height + 20,
             f'{height:.2f} M/s', ha='center', va='bottom', fontweight='bold')

# Speedup Annotation
speedup = throughput[1] / throughput[0]
plt.annotate(f'Speedup: {speedup:.2f}x', 
             xy=(0.5, 0.5), xycoords='axes fraction',
             xytext=(0, 40), textcoords='offset points',
             ha='center', fontsize=12, fontweight='bold',
             bbox=dict(boxstyle='round,pad=0.5', fc='yellow', alpha=0.5),
             arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))

# Precision Info Box
precision_text = "Precision (MAE):\nNEON: 7.52e-08\nTarget: 1e-05\nStatus: PASSED"
plt.text(0.95, 0.05, precision_text, transform=ax1.transAxes, fontsize=10,
         verticalalignment='bottom', horizontalalignment='right',
         bbox=dict(boxstyle='round', facecolor='white', alpha=0.5))

plt.tight_layout()
plt.savefig('benchmark_plot.png')
print("Plot saved to benchmark_plot.png")
