/*
    Copyright (C) 2013 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

package edu.utexas.tacc.hpcdata.structures;

import java.util.Map;
import java.util.LinkedHashMap;
import java.util.concurrent.CopyOnWriteArraySet;

public abstract class Node
{
	Node parent;
	String type;
	long i, max_index;
	Map<Long, Double> metric_map;
	CopyOnWriteArraySet<Node> children;

	public Node()
	{
		children = new CopyOnWriteArraySet<Node>();
		i=0; max_index=-1; type=null; metric_map=null; parent=null;
	}

	public void addChild(Node n)
	{
		children.add(n);
		n.setParent(this);
	}

	public void setParent(Node n)
	{
		this.parent = n;
	}

	public Node getParent()
	{
		return parent;
	}

	public CopyOnWriteArraySet<Node> getChildren()
	{
		return children;
	}

	public long getI()
	{
		return i;
	}

	public long getMaxIndex()
	{
		return max_index;
	}

	public String getType()
	{
		return type;
	}

	public boolean emptyMetricSet()
	{
		return metric_map == null || metric_map.size() == 0;
	}

	public void createEmptyMetricSet()
	{
		if (metric_map == null)
			metric_map = new LinkedHashMap<Long, Double>();
	}

	public Map<Long, Double> getAllMetrics()
	{
		return metric_map;
	}

	public double getMetric(long index)
	{
		if (!emptyMetricSet() && metric_map.containsKey(index))
			return metric_map.get(index);

		return 0;
	}

	public void setMetric(long index, double value)
	{
		if (emptyMetricSet())	createEmptyMetricSet();
		if (max_index < index)	max_index = index;

		metric_map.put(index, value);
	}

	public abstract String toString();
}
