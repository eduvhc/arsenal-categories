//! Carries the server's categories.json to the owning client. The document is a few KB, so it is
//! sent in fixed-size chunks and reassembled on the client before being handed to the config loader.
modded class SCR_PlayerController
{
	static const int ARC_CHUNK_SIZE = 900;

	protected ref array<string> m_aARC_Chunks;
	protected int m_iARC_ChunksReceived;

	//------------------------------------------------------------------------------------------------
	//! Server: push a categories JSON document to this player's client.
	void ARC_SendCategories(string json)
	{
		if (!Replication.IsServer() || json.IsEmpty())
			return;

		int length = json.Length();
		int total = (length + ARC_CHUNK_SIZE - 1) / ARC_CHUNK_SIZE;
		for (int i = 0; i < total; i++)
		{
			int start = i * ARC_CHUNK_SIZE;
			int size = Math.Min(ARC_CHUNK_SIZE, length - start);
			Rpc(ARC_RpcDo_CategoriesChunk, i, total, json.Substring(start, size));
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ARC_RpcDo_CategoriesChunk(int index, int total, string chunk)
	{
		if (total <= 0 || index < 0 || index >= total)
			return;

		if (!m_aARC_Chunks || m_aARC_Chunks.Count() != total)
		{
			m_aARC_Chunks = {};
			m_aARC_Chunks.Resize(total);
			m_iARC_ChunksReceived = 0;
		}

		if (m_aARC_Chunks[index].IsEmpty())
			m_iARC_ChunksReceived++;

		m_aARC_Chunks[index] = chunk;
		if (m_iARC_ChunksReceived < total)
			return;

		string json;
		foreach (string part : m_aARC_Chunks)
		{
			json += part;
		}

		m_aARC_Chunks = null;
		ARC_ArsenalCategoryConfig.SetServerJson(json);
		PrintFormat("[ARC] Received server categories (%1 chars)", json.Length());
	}
}
