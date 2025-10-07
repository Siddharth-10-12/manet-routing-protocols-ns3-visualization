import pandas as pd
import matplotlib.pyplot as plt

# Load packets2.csv
df = pd.read_csv("packets2.csv")

# Add a 'Second' column for aggregation
df['Second'] = df['Time'].astype(int)

# =============================
# 1. Line Plot (Tx vs Rx sizes over time)
# =============================
plt.figure()
for ttype in df['Type'].unique():
    subset = df[df['Type'] == ttype]
    plt.plot(subset['Time'], subset['Size'], label=ttype)
plt.legend()
plt.title("1. Packet Sizes over Time")
plt.xlabel("Time (s)")
plt.ylabel("Packet Size (bytes)")
plt.show()

# =============================
# 2. Bar Chart (Bytes per Second, stacked Tx vs Rx)
# =============================
packet_bytes_per_second = df.groupby(['Second', 'Type'])['Size'].sum().unstack(fill_value=0)
packet_bytes_per_second.plot(kind='bar', stacked=True)
plt.title("2. Bytes per Second (Tx vs Rx)")
plt.xlabel("Time (s)")
plt.ylabel("Total Bytes")
plt.show()

# =============================
# 3. Histogram of Packet Sizes
# =============================
plt.figure()
df['Size'].hist(bins=20)
plt.title("3. Histogram of Packet Sizes")
plt.xlabel("Packet Size (bytes)")
plt.ylabel("Frequency")
plt.show()

# =============================
# 4. Cumulative Bytes Over Time
# =============================
plt.figure()
for ttype in df['Type'].unique():
    subset = df[df['Type'] == ttype]
    plt.plot(subset['Time'], subset['Size'].cumsum(), label=f"Cumulative {ttype}")
plt.legend()
plt.title("4. Cumulative Bytes Over Time")
plt.xlabel("Time (s)")
plt.ylabel("Cumulative Bytes")
plt.show()

# =============================
# 5. Boxplot of Packet Sizes by Type
# =============================
plt.figure()
df.boxplot(column="Size", by="Type")
plt.title("5. Boxplot of Packet Sizes by Type")
plt.suptitle("")
plt.xlabel("Type")
plt.ylabel("Packet Size (bytes)")
plt.show()

# =============================
# 6. Scatter Plot (Time vs Size, colored by Type)
# =============================
plt.figure()
colors = {"Tx": "blue", "Rx": "orange"}
for ttype in df['Type'].unique():
    subset = df[df['Type'] == ttype]
    plt.scatter(subset['Time'], subset['Size'], label=ttype, alpha=0.6, c=colors[ttype])
plt.legend()
plt.title("6. Scatter Plot of Packets")
plt.xlabel("Time (s)")
plt.ylabel("Size (bytes)")
plt.show()

# =============================
# 7. Moving Average of Packet Sizes
# =============================
plt.figure()
df_sorted = df.sort_values(by="Time")
for ttype in df['Type'].unique():
    subset = df_sorted[df_sorted['Type'] == ttype]
    df_sorted.loc[subset.index, 'MovingAvg'] = subset['Size'].rolling(window=5).mean()
    plt.plot(subset['Time'], df_sorted.loc[subset.index, 'MovingAvg'], label=f"{ttype} Moving Avg")
plt.legend()
plt.title("7. Moving Average of Packet Sizes")
plt.xlabel("Time (s)")
plt.ylabel("Avg Size (bytes)")
plt.show()

# =============================
# 8. Pie Chart of Total Bytes (Tx vs Rx)
# =============================
plt.figure()
totals = df.groupby("Type")["Size"].sum()
totals.plot(kind="pie", autopct="%1.1f%%", startangle=90)
plt.title("8. Share of Bytes Sent vs Received")
plt.ylabel("")
plt.show()

# =============================
# 9. Heatmap-like Matrix (Bytes per Second)
# =============================
import seaborn as sns
plt.figure()
sns.heatmap(packet_bytes_per_second.T, cmap="coolwarm", annot=True, fmt="d")
plt.title("9. Heatmap of Bytes per Second (Tx vs Rx)")
plt.xlabel("Time (s)")
plt.ylabel("Type")
plt.show()

# =============================
# 10. Throughput Over Time (bits/sec)
# =============================
plt.figure()
bytes_per_sec = df.groupby(['Second', 'Type'])['Size'].sum() * 8  # bits/sec
for ttype in df['Type'].unique():
    plt.plot(bytes_per_sec.loc[:, ttype], label=f"{ttype} throughput")
plt.legend()
plt.title("10. Throughput Over Time")
plt.xlabel("Time (s)")
plt.ylabel("Throughput (bits/sec)")
plt.show()
