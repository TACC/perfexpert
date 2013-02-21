SELECT
recom.id AS recommendation_id,
-- normalize representative metrics
-- 0.1 represents the 'MINSUPPORT' constant
SUM(
  (CASE c.short
    WHEN 'd-l1'
    THEN (metric.perfexpert_data_accesses_L1d_hits - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'd-l2'
    THEN (metric.perfexpert_data_accesses_L2d_hits - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'd-mem'
    THEN (metric.perfexpert_data_accesses_L2d_misses - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'd-tlb'
    THEN (metric.perfexpert_data_TLB_overall - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'i-access'
    THEN (metric.perfexpert_instruction_accesses_overall - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'i-tlb'
    THEN (metric.perfexpert_instruction_TLB_overall - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'br-i'
    THEN (metric.perfexpert_branch_instructions_overall - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'fpt-fast'
    THEN (metric.perfexpert_floating_point_instr_fast_FP_instr - (maximum * 0.1))
    ELSE 0 END) +
  (CASE c.short
    WHEN 'fpt-slow'
    THEN (metric.perfexpert_floating_point_instr_slow_FP_instr - (maximum * 0.1))
    ELSE 0 END)
  ) AS score
FROM recommendation AS recom
INNER JOIN recommendation_category AS rc ON recom.id = rc.id_recommendation
INNER JOIN category AS c ON rc.id_category = c.id
JOIN metric
JOIN
  -- find the maximum between representative metrics
  (SELECT
    MAX(
      metric.perfexpert_data_accesses_L1d_hits,
      metric.perfexpert_data_accesses_L2d_hits,
      metric.perfexpert_data_accesses_L2d_misses,
      metric.perfexpert_data_TLB_overall,
      metric.perfexpert_instruction_accesses_overall,
      metric.perfexpert_instruction_TLB_overall,
      metric.perfexpert_branch_instructions_overall,
      metric.perfexpert_floating_point_instr_fast_FP_instr,
      metric.perfexpert_floating_point_instr_slow_FP_instr
    ) AS maximum
    FROM metric
    WHERE 
      -- limit the result to codes where the overall performance is higher than 1
      -- 0.5 and 1.0 represents 'good-int-CPI' and 'good-fp-CPI
      metric.perfexpert_overall * 100 / (0.5 * (100 -
      metric.perfexpert_ratio_floating_point) + 1.0 *
      metric.perfexpert_ratio_floating_point) > 1
    AND
      -- Recommender rowid, the last inserted bottleneck
      metric.id = 4
  )
WHERE recom.loop_depth >= 1
  -- Recommender rowid, the last inserted bottleneck
  AND metric.id = 4  
GROUP BY recom.id
-- ranked by their scores
ORDER BY score DESC;
