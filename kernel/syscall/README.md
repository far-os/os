# OS ABI

## Service `0x00` - Text

#### Routine `0x00` - Move cursor forward
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0000` - Service and Routine Number |

Return values:
  
  None

#### Routine `0x01` - Get cursor position
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0001` - Service and Routine Number |

Return values:
  
  | Register | Input |
  | :- | :- |
  | `cx` | Cursor position - total character number, not split into rows | 

#### Routine `0x02` - Set cursor position
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0002` - Service and Routine Number |
  | `cx` | Cursor position - total character number, not split into rows | 

Return values:
  
  None

#### Routine `0x03` - Write Character at Cursor
Parameters:

  | Register | Input |
  | :- | :- |
  | `ax` | `0x0003` - Service and Routine Number |
  | `ch` | Character in Codepage437 |
  | `cl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 

Return values:
  
  None

#### Routine `0x04` - Write String at Cursor
Parameters:

  | Register | Input |
  | - | :- |
  | `ax` | `0x0004` - Service and Routine Number |
  | `esi` | Address of null-terminated string to print |
  | `cl` | Formatting - bits 0-2 forground colour, bit 3 foreground intensity, bit 4-6 background colour, bit 7 blink | 

Return values:
  
  None
