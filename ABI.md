# OS ABI

## Service `0x00` - Text
#### Routine `0x00` - Write Cell at Cursor
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0000` - Service and Routine Number |
  | `ch` | Character in Codepage437 |
  | `cl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 

Return values:
  
  None
