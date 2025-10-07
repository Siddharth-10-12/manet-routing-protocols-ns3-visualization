import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# ------------------------------
# 1. Read CSVs and combine
# ------------------------------
protocol_files = {
    "AODV": "packets_aodv.csv",
    "DSDV": "packets_dsdv.csv",
    "OLSR": "packets_olsr2.csv",
    "DSR":  "packets_dsr2.csv"
}

# Simulated node counts for plotting
node_counts = [25, 50, 75, 100]

dfs = []
for protocol, file in protocol_files.items():
    df = pd.read_csv(file)
    df['Protocol'] = protocol

    # Simulate node counts by splitting time
    max_time = df['Time'].max()
    for i, nodes in enumerate(node_counts):
        df_subset = df[(df['Time'] >= (i*max_time/len(node_counts))) &
                       (df['Time'] < ((i+1)*max_time/len(node_counts)))].copy()
        df_subset['Nodes'] = nodes
        dfs.append(df_subset)

combined_df = pd.concat(dfs, ignore_index=True)

# ------------------------------
# 2. Aggregate packet rates
# ------------------------------
rate_df = combined_df.groupby(['Protocol','Nodes','Type'], observed=False).size().reset_index(name='Count')
rate_df['Rate'] = rate_df['Count'] / (combined_df['Time'].max()/len(node_counts))  # packets/sec approx

# ------------------------------
# 3. Line Plot: Tx/Rx Rate vs Node Count (fixed duplicate lines)
# ------------------------------
plt.figure(figsize=(10,6))
markers = {'Tx':'o','Rx':'x'}

for protocol in protocol_files.keys():
    subset = rate_df[rate_df['Protocol']==protocol]
    for pkt_type in ['Tx','Rx']:
        pkt_subset = subset[subset['Type']==pkt_type]
        plt.plot(pkt_subset['Nodes'], pkt_subset['Rate'], marker=markers[pkt_type], label=f"{protocol} {pkt_type}")

plt.title("Tx and Rx Packet Rate vs Node Count")
plt.xlabel("Number of Nodes")
plt.ylabel("Packets per Second")
plt.legend()
plt.grid(True)
plt.show()

# ------------------------------
# 4. Bar Graphs for Tx and Rx separately
# ------------------------------
# Pivot data to have protocols as columns for each node count
tx_df = rate_df[rate_df['Type']=='Tx'].pivot(index='Nodes', columns='Protocol', values='Rate').fillna(0)
rx_df = rate_df[rate_df['Type']=='Rx'].pivot(index='Nodes', columns='Protocol', values='Rate').fillna(0)

def plot_grouped_bar(df, title):
    n_nodes = len(df.index)
    n_protocols = len(df.columns)
    bar_width = 0.15
    x = np.arange(n_nodes)

    plt.figure(figsize=(10,6))
    for i, protocol in enumerate(df.columns):
        plt.bar(x + i*bar_width, df[protocol], width=bar_width, label=protocol)

    plt.xticks(x + bar_width*(n_protocols-1)/2, df.index)
    plt.xlabel("Number of Nodes")
    plt.ylabel("Packets per Second")
    plt.title(title)
    plt.legend()
    plt.grid(axis='y')
    plt.show()

plot_grouped_bar(tx_df, "Tx Packet Rate vs Node Count")
plot_grouped_bar(rx_df, "Rx Packet Rate vs Node Count")

# ------------------------------
# 5. Bar Plot: Packet Count per Protocol
# ------------------------------
plt.figure(figsize=(8,6))
sns.countplot(data=combined_df, x='Protocol', hue='Type')
plt.title("Packet Count per Protocol")
plt.show()

# ------------------------------
# 6. Line Plot: Packet Size over Time
# ------------------------------
plt.figure(figsize=(10,6))
for protocol in protocol_files.keys():
    subset = combined_df[combined_df['Protocol'] == protocol]
    plt.plot(subset['Time'], subset['Size'], label=protocol)
plt.title("Packet Size vs Time")
plt.xlabel("Time (s)")
plt.ylabel("Packet Size (bytes)")
plt.legend()
plt.show()

# ------------------------------
# 7. Scatter Plot: 50 sample points per protocol
# ------------------------------
plt.figure(figsize=(8,6))
for protocol in protocol_files.keys():
    subset = combined_df[combined_df['Protocol'] == protocol].sample(50, random_state=1)
    plt.scatter(subset['Time'], subset['Size'], label=protocol)
plt.title("Scatter Plot of 50 Packets per Protocol")
plt.xlabel("Time (s)")
plt.ylabel("Packet Size (bytes)")
plt.legend()
plt.show()

# ------------------------------
# 8. Heatmap: Packet Type counts over time bins
# ------------------------------
combined_df['TimeBin'] = pd.cut(combined_df['Time'], bins=20)
heatmap_df = combined_df.groupby(['Protocol','TimeBin','Type'], observed=False).size().reset_index(name='Count')
heatmap_pivot = heatmap_df.pivot_table(index='Type', columns=['TimeBin','Protocol'], values='Count', fill_value=0, observed=False)

plt.figure(figsize=(12,6))
sns.heatmap(heatmap_pivot, cmap='YlGnBu')
plt.title("Heatmap of Packet Counts over Time Bins per Protocol")
plt.show()

# ------------------------------
# 9. Pie Chart: Total Packets per Protocol
# ------------------------------
total_counts = combined_df['Protocol'].value_counts()
plt.figure(figsize=(6,6))
plt.pie(total_counts, labels=total_counts.index, autopct='%1.1f%%', startangle=140)
plt.title("Proportion of Packets per Protocol")
plt.show()
