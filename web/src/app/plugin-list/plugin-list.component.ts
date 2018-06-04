import { Component, OnInit, Input, OnChanges, SimpleChanges } from '@angular/core';
import { Plugin } from '../shared/plugin';
import { PluginService } from '../plugin.service';

@Component({
  selector: 'app-plugin-list',
  templateUrl: './plugin-list.component.html',
  styleUrls: ['./plugin-list.component.css']
})
export class PluginListComponent implements OnInit, OnChanges {
  constructor(
    private pluginService: PluginService) { }

  @Input() commands: string[] = [];
  plugins: Plugin[] = [];

  clear() {
    this.plugins = [];
  }
  getPluginList(commands: string[]) {
    if(commands.length > 0) {
      this.pluginService.getPluginList(commands).subscribe((result: Plugin[]) => this.plugins = result);
    }
  }
  ngOnInit() {
    this.clear();
    this.getPluginList(this.commands);
  }

  ngOnChanges(changes: SimpleChanges) {
    this.clear();
    this.getPluginList(this.commands);
  }
}
