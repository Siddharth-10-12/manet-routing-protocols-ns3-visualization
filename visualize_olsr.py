import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# -------------------- CONFIG --------------------
csv_file = "packets_olsr.csv"  # Change to packets_dsr.csv / packets_olsr.csv / packets_dsdv.csv
max_scatter_points = 50
# ------------------------------------------------

# Load CSV
df = pd.read_csv(csv_file)

# Separate Tx and Rx
tx = df[df['Type'] == 'Tx']
rx = df[df['Type'] == 'Rx']

# ---------------- BAR GRAPH ----------------
plt.figure(figsize=(6, 4))
plt.bar(['Tx', 'Rx'], [len(tx), len(rx)], color=['skyblue', 'salmon'])
plt.title('Total Tx vs Rx Packets')
plt.ylabel('Number of Packets')
plt.tight_layout()
plt.show()

# ---------------- LINE GRAPH ----------------
plt.figure(figsize=(10, 5))
plt.plot(tx['Time'], tx['Size'], color='blue', label='Tx', marker='o', markersize=3)
plt.plot(rx['Time'], rx['Size'], color='green', label='Rx', marker='x', markersize=3)
plt.title('Packet Size over Time')
plt.xlabel('Time (s)')
plt.ylabel('Packet Size (bytes)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# ---------------- SCATTER PLOT (50 points) ----------------
plt.figure(figsize=(10, 5))
plt.scatter(tx['Time'][:max_scatter_points], tx['Size'][:max_scatter_points], color='blue', label='Tx', s=40)
plt.scatter(rx['Time'][:max_scatter_points], rx['Size'][:max_scatter_points], color='green', label='Rx', s=40)
plt.title(f'Scatter Plot of First {max_scatter_points} Packets')
plt.xlabel('Time (s)')
plt.ylabel('Packet Size (bytes)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# ---------------- HEATMAP ----------------
# Bin time into 1-second intervals
df['TimeBin'] = df['Time'].astype(int)
heatmap_data = df.groupby(['TimeBin', 'Type']).size().unstack(fill_value=0)
plt.figure(figsize=(12, 6))
sns.heatmap(heatmap_data.T, annot=True, fmt="d", cmap='YlGnBu')
plt.title('Heatmap of Packets over Time')
plt.xlabel('Time Bin (s)')
plt.ylabel('Packet Type')
plt.tight_layout()
plt.show()

# ---------------- PIE CHART ----------------
plt.figure(figsize=(5, 5))
plt.pie([len(tx), len(rx)], labels=['Tx', 'Rx'], colors=['skyblue', 'salmon'], autopct='%1.1f%%', startangle=90)
plt.title('Tx vs Rx Packet Ratio')
plt.tight_layout()
plt.show()
