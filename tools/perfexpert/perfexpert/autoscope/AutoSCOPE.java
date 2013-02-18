/*
 * Copyright (c) 2013, Texas State University-San Marcos. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of the source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Texas State University-San Marcos nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT,INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING,BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Martin Burtscher
 * Version: 1.3
 */

package edu.utexas.tacc.perfexpert.autoscope;

import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.io.File;
import java.io.FileInputStream;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.Charset;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;


public class AutoSCOPE {
  private static final float THRESHOLD = 0.5f;  // recommendation cutoff
  private static final float MINSUPPORT = 0.1f;  // fraction of longest bar to be significant
  private static final String CATEGORIES = "d-L1, d-L2, d-mem, d-tlb, i-access, i-tlb, br-i, fpt-fast, fpt-slow";
  private static final int d_L1 = 0, d_L2 = 1, d_mem = 2, d_tlb = 3, i_access = 4, i_tlb = 5, br_i = 6, fpt_fast = 7, fpt_slow = 8, overall = 9;
  private static final String ATTRIBUTES = "loop1, loop2, loop3, multiple_loops, multiple_functions, boost";  // loops must go first
  private static final int loop1_attrib = 0, loop2_attrib = 1, loop3_attrib = 2, multiple_loops = 3, multiple_functions = 4, boost_attrib = 5;
  private static final String DBSTART = "BEGIN-DB", DBEND = "END-DB";

  private int[] category, cat_len;
  private int[] attribute, att_len;
  private Vector<Entry> db;
  private int[] funcloop;
  private Vector<CodeSection> cs;


  public AutoSCOPE(String dbname) {
    String PERFEXPERT_HOME = "";
    ClassLoader loader = AutoSCOPE.class.getClassLoader();
    String regex = "jar:file:(.*)/lib/perfexpert.jar!/edu/utexas/tacc/perfexpert/PerfExpert.class";
    String jarURL = loader.getResource("edu/utexas/tacc/perfexpert/PerfExpert.class").toString();

    Pattern p = Pattern.compile(regex);
    Matcher m = p.matcher(jarURL);

    if (m.find())
      PERFEXPERT_HOME = m.group(1);
    else {
      System.err.println("Could not extract location of PerfExpert jar from URL: (" +
                         jarURL + "), was using the regex: \"" + regex + "\"");
      System.exit(1);
    }

    dbname = PERFEXPERT_HOME + dbname;

    // new database
    db = new Vector<Entry>();
    funcloop = new int[2];
    Arrays.fill(funcloop, 0);
    cs = new Vector<CodeSection>();

    // process the CATEGORIES string
    category = new int[CATEGORIES.length()];
    Arrays.fill(category, -1);
    cat_len = new int[CATEGORIES.length()];
    Arrays.fill(cat_len, -1);
    String[] categories = CATEGORIES.split("\\s*,\\s*");
    assert (categories.length < 32);
    for (int i = 0; i < categories.length; i++) {
      category[CATEGORIES.indexOf(categories[i])] = i;
      cat_len[CATEGORIES.indexOf(categories[i])] = categories[i].length();
    }

    // process the ATTRIBUTES string
    attribute = new int[ATTRIBUTES.length()];
    Arrays.fill(attribute, -1);
    att_len = new int[ATTRIBUTES.length()];
    Arrays.fill(att_len, -1);
    String[] attributes = ATTRIBUTES.split("\\s*,\\s*");
    assert (attributes[0].compareTo("loop") == 0);
    assert (attributes.length < 32);
    for (int i = 0; i < attributes.length; i++) {
      attribute[ATTRIBUTES.indexOf(attributes[i])] = i;
      att_len[ATTRIBUTES.indexOf(attributes[i])] = attributes[i].length();
    }

    // read in the file and find start and end point in data
    String input = readFile(dbname);
    int beg = input.indexOf(DBSTART) + DBSTART.length();
    int end = input.indexOf(DBEND);

    // check if valid database
    if ((beg >= DBSTART.length()) && (end > beg)) {
      String database = input.substring(beg, end);

      // extract the database entries
      String[] dbentries = database.split("\\.{5,}\\s+", 0);
      for (String entry : dbentries) {
        // split each entry into its six components
        String[] component = entry.split("-{5,}\\s+", 5);
        int cat = 0;
        int attr = 0;

        // extract attributes and categories
        String[] attrcats = component[4].split("\\s+");
        for (String attrcat : attrcats) {
          if (attrcat.length() != 0) {
            // check if it's an attribute
            int index = ATTRIBUTES.indexOf(attrcat);
            if (index < 0) {
              // check if it's a category
              index = CATEGORIES.indexOf(attrcat);
              if ((index < 0) || (cat_len[index] != attrcat.length())) {
                System.err.println("unknown category or attribute: **" + attrcat + "**");
              } else {
                // include category
                cat |= 1 << category[index];
              }
            } else {
              if (att_len[index] != attrcat.length()) {
                System.err.println("unknown category or attribute: ***" + attrcat + "***");
              } else {
                // include attribute
                attr |= 1 << attribute[index];
              }
            }
          }
        }

        // count number of unique attributes
        int tmp = attr;
        int cnt = 0;
        while (tmp != 0) {
          if ((tmp & 1) != 0) cnt++;
          tmp >>>= 1;
        }
        // give higher priority to entries requiring deeper loop nests
        if ((attr & (1 << loop2_attrib)) != 0) cnt++;
        if ((attr & (1 << loop3_attrib)) != 0) cnt += 2;
        
        // remove 'boost' attribute
        attr &= ~(1 << boost_attrib);

        // add to DB if at least one category
        if (cat == 0) {
          System.err.println("need at least one category: skipping entry");
        } else {
          Entry e = new Entry(component[0], component[1], component[2], component[3], cat, attr, cnt);
          db.add(e);
        }
      }
    }
  }


  private String recommendOne(int attr, float[] weights, int max) {
    String suggestion = new String("");

    if (weights[overall] < 1.0f) {
      suggestion += "The performance of this code section is already good.";
      return suggestion;
    }
    
    // match attributes, compute score and maximum score
    float highscore = 0.0f;
    Vector<Entry> cand1 = new Vector<Entry>();
    for (Entry entry : db) {
      if (entry.Match(attr)) {
        entry.Score(weights);
        highscore = Math.max(highscore, entry.score);
        cand1.add(entry);
      }
    }

    // retain only if above cutoff
    float cutoff = highscore * THRESHOLD;
    Vector<Entry> cand2 = new Vector<Entry>();
    for (Entry entry : cand1) {
      if (entry.score > cutoff) {
        cand2.add(entry);
      }
    }

    // copy candidates into array for sorting
    Entry[] cand3 = new Entry[cand2.size()];
    cand3 = cand2.toArray(cand3);

    // sort by score, then by number of attributes, must be a stable sort
    // (currently slow bubble sort)
    int size = cand3.length;
    for (int i = size - 1; i > 0; i--) {
      for (int j = 0; j < i; j++) {
        if ((cand3[j].score < cand3[j + 1].score) || ((cand3[j].score == cand3[j + 1].score) && (cand3[j].attr_cnt < cand3[j + 1].attr_cnt))) {
          Entry temp = cand3[j];
          cand3[j] = cand3[j + 1];
          cand3[j + 1] = temp;
        }
      }
    }

    // emit up to max suggestions, all if max <= 0
    if ((max > 0) && (size > max)) {
      size = max;
    }
    suggestion += "\n";
    for (int i = 0; i < size; i++) {
      Entry entry = cand3[i];
      suggestion += entry.compilerflags;
    }
    if (suggestion.length() > 2) {
      suggestion += "--------------------------------------------------------------------------------\n\n";
    }
    for (int i = 0; i < size; i++) {
      Entry entry = cand3[i];
      suggestion += entry.description;
      if (entry.explanation.length() != 0) {
        suggestion += entry.explanation + "\n";
      }
      suggestion += entry.codeexample;
      if ((i != size - 1) && (entry.description.length()!=0 || entry.explanation.length()!=0 || entry.codeexample.length()!=0)) {
        suggestion += "--------------------------------------------------------------------------------\n\n";
      }
    }
    if (suggestion.length() <= 2) {
      suggestion += "Sorry, there are no suggestions for this code section in the database.\n";
    }
    return suggestion;
  }


  private static String readFile(String path) {
    try {
      FileInputStream stream = new FileInputStream(new File(path));
      FileChannel fc = stream.getChannel();
      MappedByteBuffer bb = fc.map(FileChannel.MapMode.READ_ONLY, 0, fc.size());
      return Charset.defaultCharset().decode(bb).toString();
    } catch (Exception e) {
      System.err.println("error accessing database file");
    }
    return "";
  }

  private class Entry {
    public String description;
    public String explanation;
    public String codeexample;
    public String compilerflags;
    public int categories;
    public int attributes; // required attributes
    public int attr_cnt;
    public float score;


    public Entry(String des, String expl, String eg, String flags, int cat, int attr, int cnt) {
      description = des;
      explanation = expl;
      codeexample = eg;
      compilerflags = flags;
      categories = cat;
      attributes = attr;
      attr_cnt = cnt;
      score = 0.0f;
    }


    public Boolean Match(int avail_attrib) {
      // check if available attributes contain all required attributes and
      // whether loop/no-loop matches
      return ((attributes & avail_attrib) == attributes) && (((attributes & 7) == 0) == ((avail_attrib & 7) == 0));
    }


    public void Score(float[] weights) {
      score = 0.0f;
      // sum up the LCPI contributions of the present categories
      for (int i = 0; i < weights.length; i++) {
        if ((categories & (1 << i)) != 0) {
          score += weights[i];
        }
      }
    }
  }


  public void addCodeSection(String header, Map<String, Float> lcpi) {
    CodeSection codesection = new CodeSection(header);

    // check if it's a loop and increment global counters accordingly
    codesection.loop = 0;
    codesection.depth = 0;
    if (header.indexOf("Loop") == 0) {
      codesection.loop = 1;
      codesection.depth = lcpi.get("loop-depth").intValue();
      if (codesection.depth > 3) {
        codesection.depth = 3;
      }
    }
    funcloop[codesection.loop]++;

    // compute category LCPIs
    Arrays.fill(codesection.weights, 0.0f);

    // XXX: Hack till we implement these two in hound
    float int_CPI = lcpi.containsKey("good-int-CPI") ? lcpi.get("good-int-CPI") : 0.5f;
    float fp_CPI  = lcpi.containsKey("good-fp-CPI") ? lcpi.get("good-fp-CPI") : 1.0f;

    codesection.weights[overall] = lcpi.get("overall") * 100.0f / (int_CPI * (100.0f - lcpi.get("ratio.floating_point")) + fp_CPI * lcpi.get("ratio.floating_point"));
    codesection.weights[d_L1] = lcpi.get("data_accesses.L1d_hits");
    codesection.weights[d_L2] = lcpi.get("data_accesses.L2d_hits");
    codesection.weights[d_mem] = lcpi.get("data_accesses.L2d_misses");
    codesection.weights[d_tlb] = lcpi.get("data_TLB.overall");
    codesection.weights[i_access] = lcpi.get("instruction_accesses.overall");
    codesection.weights[i_tlb] = lcpi.get("instruction_TLB.overall");
    codesection.weights[br_i] = lcpi.get("branch_instructions.overall");
    codesection.weights[fpt_fast] = lcpi.get("floating-point_instr.fast_FP_instr");
    codesection.weights[fpt_slow] = lcpi.get("floating-point_instr.slow_FP_instr");

    float m = codesection.weights[d_L1];
    m = Math.max(m, codesection.weights[d_L2]);
    m = Math.max(m, codesection.weights[d_mem]);
    m = Math.max(m, codesection.weights[d_tlb]);
    m = Math.max(m, codesection.weights[i_access]);
    m = Math.max(m, codesection.weights[i_tlb]);
    m = Math.max(m, codesection.weights[br_i]);
    m = Math.max(m, codesection.weights[fpt_fast]);
    m = Math.max(m, codesection.weights[fpt_slow]);

    m *= MINSUPPORT;

    codesection.weights[d_L1] -= m;
    codesection.weights[d_L2] -= m;
    codesection.weights[d_mem] -= m;
    codesection.weights[d_tlb] -= m;
    codesection.weights[i_access] -= m;
    codesection.weights[i_tlb] -= m;
    codesection.weights[br_i] -= m;
    codesection.weights[fpt_fast] -= m;
    codesection.weights[fpt_slow] -= m;
      
    cs.add(codesection);
  }

  private class CodeSection {
    public String header;
    public int loop, depth;
    public float[] weights;


    CodeSection(String name) {
      header = name;
      weights = new float[10];
      depth = 0;
    }
  }


  public String recommend(int max) {
    int attr = 0;
    if (funcloop[0] > 1) {
      attr |= 1 << multiple_functions;
    }
    if (funcloop[1] > 1) {
      attr |= 1 << multiple_loops;
    }

    String output = new String();
    for (CodeSection codesection : cs) {
      int avail_attrib = attr;
      for (int i = 0; i < codesection.depth; i++) {
        avail_attrib |= 1 << i;
      }
      output += "\n********************************************************************************\n";
      output += codesection.header;
      output += "\n********************************************************************************\n";
      output += recommendOne(avail_attrib, codesection.weights, max);
    }

    Arrays.fill(funcloop, 0);
    cs.clear();

    return output;
  }
}
