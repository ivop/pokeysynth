# PokeySynth

An LV2 Virtual Instrument that emulates the Atari Pokey soundchip.

<img src="images/pokeysynth.png" height="256">


### Channel Priorities

Channel priorities are handled according to the following table:

<table>
  <tr>
    <th colspan="1" align="center"> Priority </th>
    <th colspan="4" align="center"> Channel Configuration </th>
  </tr>
  <tr>
    <th colspan="1" align="center"> 1 </th>
    <td colspan="4" align="center"> 1+2+3+4 Linked Filtered </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 2 </th>
    <td colspan="2" align="center"> 1+3 Filtered </td>
    <td colspan="2" align="center"> 2+4 Filtered </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 3 </th>
    <td colspan="2" align="center"> 1+3 Filtered </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 4 </th>
    <td colspan="2" align="center"> 1+2 Linked </td>
    <td colspan="2" align="center"> 3+4 Linked </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 5 </th>
    <td colspan="2" align="center"> 1+2 Linked </td>
    <td colspan="1" align="center"> 3 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 6 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="2" align="center"> 2+4 Filtered </td>
    <td colspan="1" align="center"> 3 Single </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 7 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="2" align="center"> 3+4 Linked </td>
  </tr>
  <tr>
    <th colspan="1" align="center"> 8 </th>
    <td colspan="1" align="center"> 1 Single </td>
    <td colspan="1" align="center"> 2 Single </td>
    <td colspan="1" align="center"> 3 Single </td>
    <td colspan="1" align="center"> 4 Single </td>
  </tr>
</table> 

This means that, for example, a single channel instrument on channel 1, 2, and 3,
and a filtered instrument on channel 4 is handled according to priority rule 6,
which means that channel 1, 3 and 2+4 filtered are audible and the single instrument
on channel 2 is muted.
