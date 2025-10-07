import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
try:
    df = pd.read_csv("packets.csv")
except FileNotFoundError:
    print("Error: packets.csv not found")
    exit()

# Ensure correct types and remove invalid rows
df['Time'] = pd.to_numeric(df['Time'], errors='coerce')
df['Size'] = pd.to_numeric(df['Size'], errors='coerce')
df = df.dropna()

# Separate Tx and Rx
tx = df[df['Type'] == 'Tx']
rx = df[df['Type'] == 'Rx']

# Round time to nearest second
df['Second'] = df['Time'].astype(int)

# 1️⃣ Stacked Bar: Packet sizes per second (Tx + Rx)
packet_bytes_per_second = df.groupby(['Second', 'Type'])['Size'].sum().unstack(fill_value=0)

# 2️⃣ Cumulative packet sizes
cumulative_tx = tx['Size'].cumsum()
cumulative_rx = rx['Size'].cumsum()

# 3️⃣ Average packet size per second
avg_size_per_sec = df.groupby(['Second', 'Type'])['Size'].mean().unstack(fill_value=0)

# 4️⃣ Tx/Rx ratio per second
packets_per_second_count = df.groupby(['Second', 'Type']).size().unstack(fill_value=0)
ratio = (packets_per_second_count['Tx'] / packets_per_second_count['Rx']).replace([float('inf'), float('nan')], 0)

# Create subplots
fig, axs = plt.subplots(3, 2, figsize=(15, 15))
fig.suptitle("NS-3 Packet Analysis Visualizations", fontsize=16)

# 1️⃣ Line plot: Packet sizes over time
axs[0,0].plot(tx['Time'], tx['Size'], label='Tx', marker='o', linestyle='-')
axs[0,0].plot(rx['Time'], rx['Size'], label='Rx', marker='x', linestyle='--')
axs[0,0].set_xlabel("Time (s)")
axs[0,0].set_ylabel("Packet Size (bytes)")
axs[0,0].set_title("Packet Sizes Over Time")
axs[0,0].legend()
axs[0,0].grid(True)

# 2️⃣ Stacked Bar: Packet sizes per second
packet_bytes_per_second.plot(kind='bar', stacked=True, ax=axs[0,1])
axs[0,1].set_xlabel("Second")
axs[0,1].set_ylabel("Total Packet Size (bytes)")
axs[0,1].set_title("Packet Sizes Per Second (Stacked Tx + Rx)")
axs[0,1].grid(axis='y')

# 3️⃣ Cumulative packet sizes
axs[1,0].plot(tx['Time'], cumulative_tx, label='Cumulative Tx', color='blue')
axs[1,0].plot(rx['Time'], cumulative_rx, label='Cumulative Rx', color='orange')
axs[1,0].set_xlabel("Time (s)")
axs[1,0].set_ylabel("Cumulative Bytes")
axs[1,0].set_title("Cumulative Packet Sizes")
axs[1,0].legend()
axs[1,0].grid(True)

# 4️⃣ Average packet size per second
avg_size_per_sec.plot(kind='bar', stacked=True, ax=axs[1,1])
axs[1,1].set_xlabel("Second")
axs[1,1].set_ylabel("Average Packet Size (bytes)")
axs[1,1].set_title("Average Packet Size Per Second")
axs[1,1].grid(axis='y')

# 5️⃣ Histogram of packet sizes
axs[2,0].hist([tx['Size'], rx['Size']], bins=10, label=['Tx','Rx'], color=['blue','orange'], alpha=0.7)
axs[2,0].set_xlabel("Packet Size (bytes)")
axs[2,0].set_ylabel("Frequency")
axs[2,0].set_title("Histogram of Packet Sizes")
axs[2,0].legend()
axs[2,0].grid(True)

# 6️⃣ Tx/Rx ratio per second
axs[2,1].plot(ratio.index, ratio.values, marker='o', color='green')
axs[2,1].set_xlabel("Second")
axs[2,1].set_ylabel("Tx/Rx Ratio")
axs[2,1].set_title("Tx/Rx Ratio Over Time")
axs[2,1].grid(True)

plt.tight_layout(rect=[0, 0, 1, 0.96])
plt.show()
